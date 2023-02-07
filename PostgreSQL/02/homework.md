# 02. SQL и реляционные СУБД. Введение в PostgreSQL

>выключить auto commit
iso=# \set AUTOCOMMIT OFF
iso=# \echo :AUTOCOMMIT
OFF

>сделать в первой сессии новую таблицу и наполнить ее данными
create table persons(id serial, first_name text, second_name text);
insert into persons(first_name, second_name) values('ivan', 'ivanov');
insert into persons(first_name, second_name) values('petr', 'petrov');
commit;
iso=# create table persons(id serial, first_name text, second_name text);
insert into persons(first_name, second_name) values('ivan', 'ivanov');
insert into persons(first_name, second_name) values('petr', 'petrov');
commit;
CREATE TABLE
INSERT 0 1
INSERT 0 1
COMMIT

>посмотреть текущий уровень изоляции: show transaction isolation level
iso=# show transaction isolation level;
 transaction_isolation
-----------------------
 read committed
(1 row)

>начать новую транзакцию в обоих сессиях с дефолтным (не меняя) уровнем изоляции

>в первой сессии добавить новую запись
>insert into persons(first_name, second_name) values('sergey', 'sergeev');
iso=*# insert into persons(first_name, second_name) values('sergey', 'sergeev');
INSERT 0 1
>сделать select * from persons во второй сессии
iso=*# select * from persons;
 id | first_name | second_name
----+------------+-------------
  1 | ivan       | ivanov
  2 | petr       | petrov
(2 rows)
>видите ли вы новую запись и если да то почему?
	Новая запись в выборку не попала, так как транзакция не зафиксирована, а postgres не допускает грязное чтение
> завершить первую транзакцию - commit;

> сделать select * from persons во второй сессии
iso=*# select * from persons;
 id | first_name | second_name
----+------------+-------------
  1 | ivan       | ivanov
  2 | petr       | petrov
  3 | sergey     | sergeev
(3 rows)
> видите ли вы новую запись и если да то почему?
	Да, новая запись теперь попала в выборку, так как транзакция зафиксирована.
> завершите транзакцию во второй сессии
> начать новые но уже repeatable read транзакции - set transaction isolation level repeatable read;
> в первой сессии добавить новую запись
insert into persons(first_name, second_name) values('sveta', 'svetova');
> сделать select * from persons во второй сессии
iso=*# select * from persons;
 id | first_name | second_name
----+------------+-------------
  1 | ivan       | ivanov
  2 | petr       | petrov
  3 | sergey     | sergeev
(3 rows)
> видите ли вы новую запись и если да то почему?
	Новая запись в выборку не попала, так как транзакция не зафиксирована, а postgres не допускает грязное чтение
> завершить первую транзакцию - commit;
> сделать select * from persons во второй сессии
iso=*# select * from persons;
 id | first_name | second_name
----+------------+-------------
  1 | ivan       | ivanov
  2 | petr       | petrov
  3 | sergey     | sergeev
(3 rows)
> видите ли вы новую запись и если да то почему?
	Новая запись в выборку не попала, так как в текущей транзакции уже ранее было обращение к таблице persons и поэтому изоляция «repeatable read» отображает снимок таблицы persons на момент первого обращения в транзакции;
> завершить вторую транзакцию
> сделать select * from persons во второй сессии
iso=# select * from persons;
 id | first_name | second_name
----+------------+-------------
  1 | ivan       | ivanov
  2 | petr       | petrov
  3 | sergey     | sergeev
  4 | sveta      | svetova
(4 rows)
> видите ли вы новую запись и если да то почему?
	При первом обращении к таблице persons в рамках текущей транзакции, первая транзакция уже зафиксирована и поэтому вторая транзакция в новом снимке видит новую строку

