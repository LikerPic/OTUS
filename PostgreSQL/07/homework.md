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
```
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
```
## 15 сделайте select * from t1;
```console
```
## 16 получилось? (могло если вы делали сами не по шпаргалке и не упустили один существенный момент про который позже)

## 17 напишите что именно произошло в тексте домашнего задания
## 18 у вас есть идеи почему? ведь права то дали?
## 19 посмотрите на список таблиц
```console
```
## 20 подсказка в шпаргалке под пунктом 20
## 21 а почему так получилось с таблицей (если делали сами и без шпаргалки то может у вас все нормально)
## 22 вернитесь в базу данных testdb под пользователем postgres
```console
```
## 23 удалите таблицу t1
```console
```
## 24 создайте ее заново но уже с явным указанием имени схемы testnm
```console
```
## 25 вставьте строку со значением c1=1
```console
```
## 26 зайдите под пользователем testread в базу данных testdb
```console
```
## 27 сделайте select * from testnm.t1;
```console
```
## 28 получилось?
## 29 есть идеи почему? если нет - смотрите шпаргалку
## 30 как сделать так чтобы такое больше не повторялось? если нет идей - смотрите шпаргалку
## 31 сделайте select * from testnm.t1;
```console
```
## 32 получилось?
## 33 есть идеи почему? если нет - смотрите шпаргалку
## 31 сделайте select * from testnm.t1;
```console
```
## 32 получилось?
## 33 ура!
## 34 теперь попробуйте выполнить команду create table t2(c1 integer); insert into t2 values (2);
```console
```
## 35 а как так? нам же никто прав на создание таблиц и insert в них под ролью readonly?
## 36 есть идеи как убрать эти права? если нет - смотрите шпаргалку
## 37 если вы справились сами то расскажите что сделали и почему, если смотрели шпаргалку - объясните что сделали и почему выполнив указанные в ней команды
## 38 теперь попробуйте выполнить команду create table t3(c1 integer); insert into t2 values (2);
```console
```
## 39 расскажите что получилось и почему


Ссылки по теме:
https://postgrespro.ru/docs/postgresql/14/sql-grant
https://www.postgresql.org/docs/current/app-psql.html
https://www.postgresql.org/docs/current/sql-createrole.html
https://www.postgresql.org/docs/current/sql-createuser.html
https://postgrespro.ru/docs/postgresql/14/sql-alteruser
