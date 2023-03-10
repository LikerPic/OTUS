# 10. Блокировки

```diff
+Имеем VM VirtualBox с Ubuntu 22.04 + Docker (postgres:14)
```

## 1. Настройте сервер так, чтобы в журнал сообщений сбрасывалась информация о блокировках, удерживаемых более 200 миллисекунд. Воспроизведите ситуацию, при которой в журнале появятся такие сообщения.


Настройки блокировок:
```console
postgres=# select name,setting,unit,short_desc from pg_settings where name similar to 'lock%|%\_lock%|%deadlock%|%n\_statement';

              name              | setting | unit |                                 short_desc
--------------------------------+---------+------+----------------------------------------------------------------------------
 deadlock_timeout               | 1000    | ms   | Задаёт интервал ожидания в блокировке до проверки на взаимоблокировку.
 lock_timeout                   | 0       | ms   | Задаёт максимальную продолжительность ожидания блокировок.
 log_lock_waits                 | off     |      | Протоколировать длительные ожидания в блокировках.
 log_min_duration_statement     | -1      | ms   | Задаёт предельное время выполнения любого оператора, при превышении которого он выводится в журнал.
 max_locks_per_transaction      | 64      |      | Задаёт предельное число блокировок на транзакцию.
 max_pred_locks_per_page        | 2       |      | Задаёт максимальное число кортежей, блокируемых предикатными блокировками в одной странице.
 max_pred_locks_per_relation    | -2      |      | Задаёт максимальное число страниц и кортежей, блокируемых предикатными блокировками в одном отношении.
 max_pred_locks_per_transaction | 64      |      | Задаёт предельное число предикатных блокировок на транзакцию.
(8 rows)
```
из интересного:
- `deadlock_timeout` - предельное время ожидания блокировки для срабатывания проверки на deadlock ИЛИ записи в лог;
- `log_lock_waits` - рпизнак записи в лог длительного ожидания блокировки > `deadlock_timeout`.

Выставим следующие настройки кластера:
```diff
+> logging_collector = on
+> log_lock_waits = on
+> deadlock_timeout = 200ms
```

Запустим кластер.
**ТЕРМИНАЛ 1**:
```console
postgres=# select name,setting,unit from pg_settings where name IN ('deadlock_timeout','log_lock_waits');
       name       | setting | unit
------------------+---------+------
 deadlock_timeout | 200     | ms
 log_lock_waits   | on      |
(2 rows)

postgres=# create database lock;
CREATE DATABASE
postgres=# \c lock
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "lock" as user "postgres".
lock=# create table tbl_lesson10(lucky_num int);
CREATE TABLE
lock=# insert into tbl_lesson10 VALUES (555);
INSERT 0 1
lock=# BEGIN;
BEGIN
lock=*# update tbl_lesson10 SET lucky_num=777;
UPDATE 1
```

Запускаем **ТЕРМИНАЛ 2** (PID=`111`)
```console
lock=# select pg_backend_pid();
 pg_backend_pid
----------------
            111
(1 row)

postgres=# \c lock
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "lock" as user "postgres".
lock=# BEGIN;
BEGIN
lock=*# update tbl_lesson10 SET lucky_num=888;
```
на этом моменте ТЕРМИНАЛ 2 повисает в ожидании блокировки.

Завершаем транзакцию в ТЕРМИНАЛ 1:
```console
lock=*# commit;
COMMIT
```
После этого отвисает ТЕРМИНАЛ 2 и можно завершать транзакцию:
```console
UPDATE 1
lock=*# commit;
COMMIT
```

В логе наблюдаем сообщение о том, что процесс PID=`111` ожидает блокировку 204 мс:
```console
root@40d695990f10:/var/lib/postgresql/data/log# tail -n 4 postgresql-2023-03-10_150345.log
2023-03-10 15:12:36.343 GMT [111] LOG:  process 111 still waiting for ShareLock on transaction 737 after 204.763 ms
2023-03-10 15:12:36.343 GMT [111] DETAIL:  Process holding the lock: 82. Wait queue: 111.
2023-03-10 15:12:36.343 GMT [111] CONTEXT:  while updating tuple (0,1) in relation "tbl_lesson10"
2023-03-10 15:12:36.343 GMT [111] STATEMENT:  update tbl_lesson10 SET lucky_num=888;
```


## 2. Смоделируйте ситуацию обновления одной и той же строки тремя командами UPDATE в разных сеансах. Изучите возникшие блокировки в представлении pg_locks и убедитесь, что все они понятны. Пришлите список блокировок и объясните, что значит каждая.

Подготовим таблицу `clients` и представление для `pg_locks`
```console
lock=# create table clients (id int PRIMARY KEY, name varchar(200));
CREATE TABLE
lock=# insert into clients VALUES (1,'Nick');
INSERT 0 1
lock=# insert into clients VALUES (2,'Helen');
INSERT 0 1
lock=# select * from clients ;
 id | name
----+-------
  1 | Nick
  2 | Helen
(2 rows)

lock=# CREATE VIEW locks_v AS
SELECT pid,
       locktype,
       CASE locktype
         WHEN 'relation' THEN relation::regclass::text
         WHEN 'transactionid' THEN transactionid::text
         WHEN 'tuple' THEN relation::regclass::text||':'||tuple::text
       END AS lockid,
       mode,
       granted
FROM pg_locks
WHERE locktype in ('relation','transactionid','tuple')
AND (locktype != 'relation' OR relation = 'clients'::regclass);
CREATE VIEW
```


|Команда|ТЕРМИНАЛ 1|ТЕРМИНАЛ 2|ТЕРМИНАЛ 3|
|-|-|-|-|
|BEGIN;|BEGIN|BEGIN|BEGIN|
|txid_current()| 743|749 | 750|
|pg_backend_pid()| 82|217 | 218|
|update ... |UPDATE 1|Повис|Повис|


После открытия транзакции видим появление трех (для каждого процесса своя) экслюзивных блокировок на объет типа `Транзакция`:
```sql
lock=*# select * from locks_v ;
 pid |   locktype    | lockid |     mode      | granted
-----+---------------+--------+---------------+---------
  82 | transactionid | 747    | ExclusiveLock | t
 218 | transactionid | 750    | ExclusiveLock | t
 217 | transactionid | 749    | ExclusiveLock | t
(3 rows)
```
---
ТЕРМИНАЛ 1 (PID=82): `update clients SET name='John' where id=1;`

Видим добавление одной блокировки:
- на таблицу `clients`;
```sql
lock=*# select * from locks_v ;
 pid |   locktype    | lockid  |       mode       | granted
-----+---------------+---------+------------------+---------
  82 | relation      | clients | RowExclusiveLock | t
  82 | transactionid | 747     | ExclusiveLock    | t
 217 | transactionid | 749     | ExclusiveLock    | t
 218 | transactionid | 750     | ExclusiveLock    | t
(4 rows)
```
---
ТЕРМИНАЛ 2 (PID=217): `update clients SET name='Bob' where id=1;`

Видим добавление трех блокировок:
- на таблицу `clients` (как у первого процесса);
- на строку 1 (tuple) таблицы `clients`;
- на транзакцию 747 первого процесса (неуспешная блокировка)
```sql
lock=*# select * from locks_v ;
 pid |   locktype    |  lockid   |       mode       | granted
-----+---------------+-----------+------------------+---------
  82 | relation      | clients   | RowExclusiveLock | t
  82 | transactionid | 747       | ExclusiveLock    | t
 217 | relation      | clients   | RowExclusiveLock | t
 217 | tuple         | clients:1 | ExclusiveLock    | t
 217 | transactionid | 749       | ExclusiveLock    | t
 217 | transactionid | 747       | ShareLock        | f
 218 | transactionid | 750       | ExclusiveLock    | t
(7 rows)
```
---
ТЕРМИНАЛ 3 (PID=218): `update clients SET name='Alice' where id=1;`

Видим добавление двух блокировок:
- на таблицу `clients` (как у первого процесса);
- на строку id=1 (tuple) таблицы `clients` (как у второго процесса) - пока неуспешный захват, т.к. блокировка экслюзивная;
```sql
lock=# select * from locks_v ;
 pid |   locktype    |  lockid   |       mode       | granted
-----+---------------+-----------+------------------+---------
  82 | relation      | clients   | RowExclusiveLock | t
  82 | transactionid | 747       | ExclusiveLock    | t
 217 | relation      | clients   | RowExclusiveLock | t
 217 | tuple         | clients:1 | ExclusiveLock    | t
 217 | transactionid | 749       | ExclusiveLock    | t
 217 | transactionid | 747       | ShareLock        | f
 218 | tuple         | clients:1 | ExclusiveLock    | f
 218 | relation      | clients   | RowExclusiveLock | t
 218 | transactionid | 750       | ExclusiveLock    | t
(9 rows)
```
---
ТЕРМИНАЛ 1 (PID=82): `commit;`

После этого:
- процесс `82` (терминал 2) снимает обе блокировки и освобождает транзакцию для второго процесса;
- второй процесс снял две блокировки: на чужую транзакцию и на строку id=1;
- теперь третий процесс ожидает освобождения транзакции от второго процесса вместо ожидания строки id=1;
- второй процесс просыпается и выполняет команду UPDATE.
```sql
lock=# select * from locks_v ;
 pid |   locktype    | lockid  |       mode       | granted
-----+---------------+---------+------------------+---------
 217 | relation      | clients | RowExclusiveLock | t
 217 | transactionid | 749     | ExclusiveLock    | t
 218 | relation      | clients | RowExclusiveLock | t
 218 | transactionid | 749     | ShareLock        | f
 218 | transactionid | 750     | ExclusiveLock    | t
(5 rows)
```

---
ТЕРМИНАЛ 2 (PID=217): `commit;`

После этого:
- второй процесс снимает обе оставшиеся блокировки;
- третий процесс снимает блокировку на транзакцию второго процесса и просыпается
```sql
lock=# select * from locks_v ;
 pid |   locktype    | lockid  |       mode       | granted
-----+---------------+---------+------------------+---------
 218 | relation      | clients | RowExclusiveLock | t
 218 | transactionid | 750     | ExclusiveLock    | t
(2 rows)
```

---
ТЕРМИНАЛ 3 (PID=218): `commit;`
Все блокировки сняты:
```sql
lock=# select * from locks_v ;
 pid | locktype | lockid | mode | granted
-----+----------+--------+------+---------
(0 rows)
```


## 3. Воспроизведите взаимоблокировку трех транзакций. Можно ли разобраться в ситуации постфактум, изучая журнал сообщений?

## 4. Могут ли две транзакции, выполняющие единственную команду UPDATE одной и той же таблицы (без where), заблокировать друг друга?





---
**КОМАНДЫ**
|Описание|Команда|
|-|-|
|Настройки блокировок|select name,setting,unit,short_desc from pg_settings where name similar to 'lock%|%\_lock%|%deadlock%|%n\_statement';|

---
**ССЫЛКИ**
|Описание|Ссылка|
|-|-|
