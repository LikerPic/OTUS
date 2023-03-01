# 07. Логический уровень PostgreSQL 

## 1 создайте новый кластер PostgresSQL 14
```console
vboxuser@Ubuntu22:~$ docker run --name pg_lesson7 -e POSTGRES_PASSWORD=postgres -d -p 6432:5432 postgres:14
6efbfa82476096b94019c135d6260d57f63581d1da7f350c6ec76b6ffe4203bf
```
## 2 зайдите в созданный кластер под пользователем postgres
```console
vboxuser@Ubuntu22:~$ psql -h localhost -p 6432 -U postgres
Password for user postgres:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
Type "help" for help.
```
## 3 создайте новую базу данных testdb
```console
postgres=# create database testdb;
CREATE DATABASE
```
## 4 зайдите в созданную базу данных под пользователем postgres
```console
postgres=# \c testdb
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "postgres".
```
## 5 создайте новую схему testnm
```console
testdb=# create schema testnm;
CREATE SCHEMA
testdb=# \dn
  List of schemas
  Name  |  Owner
--------+----------
 public | postgres
 testnm | postgres
(2 rows)
```
## 6 создайте новую таблицу t1 с одной колонкой c1 типа integer
```console
testdb=# create table t1 (c1 int);
CREATE TABLE
```
## 7 вставьте строку со значением c1=1
```console
testdb=# insert into t1 values (1);
INSERT 0 1
```
## 8 создайте новую роль readonly
```console
testdb=# create role readonly;
CREATE ROLE
testdb=# \du
                                   List of roles
 Role name |                         Attributes                         | Member of
-----------+------------------------------------------------------------+-----------
 postgres  | Superuser, Create role, Create DB, Replication, Bypass RLS | {}
 readonly  | Cannot login                                               | {}
```
## 9 дайте новой роли право на подключение к базе данных testdb
```console
testdb=# grant connect on database testdb to readonly;
GRANT
```
В писке баз теперь витдим упоминание пользователя `readonly` с доступом `c` (CONNECT):
```console
testdb=# \l testdb
                                               List of databases
  Name  |  Owner   | Encoding |  Collate   |   Ctype    | ICU Locale | Locale Provider |   Access privileges
--------+----------+----------+------------+------------+------------+-----------------+-----------------------
 testdb | postgres | UTF8     | en_US.utf8 | en_US.utf8 |            | libc            | =Tc/postgres         +
        |          |          |            |            |            |                 | postgres=CTc/postgres+
        |          |          |            |            |            |                 | readonly=c/postgres
(1 row)
```

## 10 дайте новой роли право на использование схемы testnm
```console
testdb=# grant usage on schema testnm to readonly;
GRANT
```
Теперь для схемы `testnm` видим доступ для пользователя `readonly` с доступом `U` (USAGE):
```console
testdb=# \dn+
                          List of schemas
  Name  |  Owner   |  Access privileges   |      Description
--------+----------+----------------------+------------------------
 public | postgres | postgres=UC/postgres+| standard public schema
        |          | =UC/postgres         |
 testnm | postgres | postgres=UC/postgres+|
        |          | readonly=U/postgres  |
(2 rows)
```
## 11 дайте новой роли право на select для всех таблиц схемы testnm
```console
testdb=# grant select on all tables in schema testnm to readonly;
GRANT
```
Изменений в описании доступов к схеме не видно (виджимо это проверяется на конкретной таблице, а не на схеме):
```console
testdb=# \dn+
                          List of schemas
  Name  |  Owner   |  Access privileges   |      Description
--------+----------+----------------------+------------------------
 public | postgres | postgres=UC/postgres+| standard public schema
        |          | =UC/postgres         |
 testnm | postgres | postgres=UC/postgres+|
        |          | readonly=U/postgres  |
(2 rows)
```
## 12 создайте пользователя testread с паролем test123
```console
testdb=# create user testread with password 'test123';
CREATE ROLE
testdb=# \du
                                   List of roles
 Role name |                         Attributes                         | Member of
-----------+------------------------------------------------------------+-----------
 postgres  | Superuser, Create role, Create DB, Replication, Bypass RLS | {}
 readonly  | Cannot login                                               | {}
 testread  |                                                            | {}
```
## 13 дайте роль readonly пользователю testread
```console
testdb=# grant readonly to testread;
GRANT ROLE
```
Видим изменения в графе `Member of` для пользователя `readonly`:
```console
testdb=# \du
                                    List of roles
 Role name |                         Attributes                         | Member of
-----------+------------------------------------------------------------+------------
 postgres  | Superuser, Create role, Create DB, Replication, Bypass RLS | {}
 readonly  | Cannot login                                               | {}
 testread  |                                                            | {readonly}
```
## 14 зайдите под пользователем testread в базу данных testdb
```console
testdb=# \c testdb testread
Password for user testread:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "testread".
```
## 15 сделайте select * from t1;
```console
testdb=> select * from t1;
ERROR:  permission denied for table t1
```
## 16 получилось? (могло если вы делали сами не по шпаргалке и не упустили один существенный момент про который позже)
Не получилось.
## 17 напишите что именно произошло в тексте домашнего задания
Пользователь `testread` попытался обратиться к таблице `t1`. При этом таблица была создана в схеме `public`, на которую у пользователя нет доступа.

## 18 у вас есть идеи почему? ведь права то дали?
Пользователю `testread` дали права на все таблицы схемы `testnm`, но не дали права на схему `public`. Для доступа к таблице требуется 
- либо дать пользователю права на схему `public`;
- либо перенести таблицу `t1` в схему `testnm`.

## 19 посмотрите на список таблиц
```console
testdb=> \dt
        List of relations
 Schema | Name | Type  |  Owner
--------+------+-------+----------
 public | t1   | table | postgres
(1 row)
```
## 20 подсказка в шпаргалке под пунктом 20
## 21 а почему так получилось с таблицей (если делали сами и без шпаргалки то может у вас все нормально)
Если при создании таблицы не указать схему, то будет применяться текущая по умолчанию. Схема по умолчанию на момент создания таблицы была `public`
```console
testdb=> SELECT current_schema();
 current_schema
----------------
 public
(1 row)
```

## 22 вернитесь в базу данных testdb под пользователем postgres
```console
testdb=> \c testdb postgres
Password for user postgres:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "postgres".
```
## 23 удалите таблицу t1
```console
testdb=# drop table t1;
DROP TABLE
testdb=# \dt
Did not find any relations.
```
## 24 создайте ее заново но уже с явным указанием имени схемы testnm
```console
testdb=# create table testnm.t1 (c1 int);
CREATE TABLE
testdb=# \dt
Did not find any relations.
```
Таблица создалась, при этом `\dt` её не показывает, как в прочем и простой селект:
```console
testdb=# select * from t1;
ERROR:  relation "t1" does not exist
LINE 1: select * from t1;
```
Видимо из-за того, что запросы идут в текущей схеме `public`. То есть важно как момент `CREATE` понимать в какой схеме создается объет, так и во время `SELECT` важно понимать в какой конкретно схеме запрашиваем данные.
## 25 вставьте строку со значением c1=1
```console
testdb=# insert into testnm.t1 values(1);
INSERT 0 1
```
## 26 зайдите под пользователем testread в базу данных testdb
```console
testdb=# \c testdb testread
Password for user testread:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "testread".
```
## 27 сделайте select * from testnm.t1;
```console
testdb=> select * from testnm.t1;
ERROR:  permission denied for table t1
```
## 28 получилось?
Не получилось - ошибюка доступа.
## 29 есть идеи почему? если нет - смотрите шпаргалку
Запрос `\dt` показывает, что таблица принадлежит схеме testnm.
```console
testdb=> \dt
Did not find any relations.
testdb=> \dt t1
Did not find any relation named "t1".
testdb=> \dt testnm.t1
        List of relations
 Schema | Name | Type  |  Owner
--------+------+-------+----------
 testnm | t1   | table | postgres
(1 row)
```
Предположу две проблемы:
1. Доступ в виде `all tables in schema testnm` применился тогда, когда таблица еще не была создана. Другими словами, такой доступ работает на существующие объекты, но не на вновь создаваемые.
2. Нужно принудительно изменить текущую схему по умолчанию, либо порядок просмотра схем.
(в ответах совпал п.1)

## 30 как сделать так чтобы такое больше не повторялось? если нет идей - смотрите шпаргалку
Предположу два варианта:
- либо явно раздавать доступ на таблицу после каждого создания таблицы;
- либо предусмотреть доступ по маске вида testnm.* (если такое поддерживается)
В ответе: `ALTER default privileges in SCHEMA testnm grant SELECT on TABLEs to readonly; `
Выполним подсказку:
```console
testdb=> \c testdb postgres
Password for user postgres:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "postgres".
testdb=# ALTER default privileges in SCHEMA testnm grant SELECT on TABLEs to readonly;
ALTER DEFAULT PRIVILEGES
testdb=# \c testdb testread
Password for user testread:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "testread".
```

## 31 сделайте select * from testnm.t1;
```console
testdb=> select * from testnm.t1;
ERROR:  permission denied for table t1
```
## 32 получилось?
Не получилось.
## 33 есть идеи почему? если нет - смотрите шпаргалку
Видимо `ALTER DEFAULT PRIVILEGES` будет отрабатывать на вновь создаваемые объекты. На момент изменения доступа по умолчанию таблица `t1` уже существовала.
(совпало с ответом)
Пересоздадим таблицу `t1`:
```console
testdb=> drop table testnm.t1;
ERROR:  must be owner of table t1
testdb=> \c testdb postgres
Password for user postgres:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "postgres".
testdb=# drop table testnm.t1;
DROP TABLE
testdb=# create table testnm.t1 (c1 int);
CREATE TABLE
testdb=# insert into testnm.t1 values(1);
INSERT 0 1
```
## 31 сделайте select * from testnm.t1;
```console
testdb=# \c testdb testread
Password for user testread:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "testread".
testdb=> select * from testnm.t1;
 c1
----
  1
(1 row)
```
## 32 получилось?
Получилось.

## 33 ура!
Ура!
## 34 теперь попробуйте выполнить команду create table t2(c1 integer); insert into t2 values (2);
```console
testdb=> create table t2(c1 integer); insert into t2 values (2);
CREATE TABLE
INSERT 0 1
```
## 35 а как так? нам же никто прав на создание таблиц и insert в них под ролью readonly?
```console
testdb=> \dt
        List of relations
 Schema | Name | Type  |  Owner
--------+------+-------+----------
 public | t2   | table | testread
(1 row)

testdb=> \dn+
                          List of schemas
  Name  |  Owner   |  Access privileges   |      Description
--------+----------+----------------------+------------------------
 public | postgres | postgres=UC/postgres+| standard public schema
        |          | =UC/postgres         |
 testnm | postgres | postgres=UC/postgres+|
        |          | readonly=U/postgres  |
(2 rows)
```
Таблица создана в схеме `public`. Судя по всему для этой схемы доступ по умолчанию дается всем на всё при добавлении новых объектов. Либо же дело в том, что `testread` как владелец получил полный доступ на таблицу `t2`.
## 36 есть идеи как убрать эти права? если нет - смотрите шпаргалку
Предположу:
- либо сменить владельца у таблицы `t2`;
- либо изменить доступ по умолчанию `ALTER DEFAULT PRIVILEGES` в схеме `public` (будет применяться для вновь созданных объектов).

Ответ:
> это все потому что search_path указывает в первую очередь на схему public. А схема public создается в каждой базе данных по умолчанию. И grant на все действия в этой схеме дается роли public. А роль public добавляется всем новым пользователям. Соответсвенно каждый пользователь может по умолчанию создавать объекты в схеме public любой базы данных, ес-но если у него есть право на подключение к этой базе данных. Чтобы раз и навсегда забыть про роль public - а в продакшн базе данных про нее лучше забыть - выполните следующие действия 
```console
\c testdb postgres; 
revoke CREATE on SCHEMA public FROM public; 
revoke all on DATABASE testdb FROM public; 
\c testdb testread; 
```
Выполняем подсказку:
```console
testdb=> \c testdb postgres
Password for user postgres:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "postgres".
testdb=# revoke CREATE on SCHEMA public FROM public;
REVOKE
testdb=# revoke all on DATABASE testdb FROM public;
REVOKE
testdb=# \c testdb testread;
Password for user testread:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
You are now connected to database "testdb" as user "testread".
```

## 37 если вы справились сами то расскажите что сделали и почему, если смотрели шпаргалку - объясните что сделали и почему выполнив указанные в ней команды
Первая команда отзывает доступ на `CREATE` в схеме `public` у роли `public`;

Вторая команда отзывает все доступы в БД `testdb` у роли `public`.

## 38 теперь попробуйте выполнить команду create table t3(c1 integer); insert into t2 values (2);
```console
testdb=> create table t3(c1 integer); insert into t2 values (2);
ERROR:  permission denied for schema public
LINE 1: create table t3(c1 integer);
                     ^
INSERT 0 1
```
## 39 расскажите что получилось и почему
На этот раз создать новую таблицу создать не удалось из-за отказа в доступе, т.к. доступ на создание таблиц ранее был отозван `revoke CREATE` в схеме `public`.

В таблицу t2 успешно вставилась строка. Доступ на это есть как владельцу таблицы.



Ссылки по теме:

https://postgrespro.ru/docs/postgresql/14/sql-grant
https://www.postgresql.org/docs/current/app-psql.html
https://www.postgresql.org/docs/current/sql-createrole.html
https://www.postgresql.org/docs/current/sql-createuser.html
https://postgrespro.ru/docs/postgresql/14/sql-alteruser
