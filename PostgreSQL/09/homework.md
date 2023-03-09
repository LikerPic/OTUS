
# **09. Журналы**
```diff
+Имеем VM VirtualBox с Ubuntu 22.04 + Docker
```

## 1. Настройте выполнение контрольной точки раз в 30 секунд.

## 2. 10 минут c помощью утилиты pgbench подавайте нагрузку.

## 3. Измерьте, какой объем журнальных файлов был сгенерирован за это время. Оцените, какой объем приходится в среднем на одну контрольную точку.

## 4. Проверьте данные статистики: все ли контрольные точки выполнялись точно по расписанию. Почему так произошло?

## 5. Сравните tps в синхронном/асинхронном режиме утилитой pgbench. Объясните полученный результат.

## 6. Создайте новый кластер с включенной контрольной суммой страниц. Создайте таблицу. Вставьте несколько значений. Выключите кластер. Измените пару байт в таблице. Включите кластер и сделайте выборку из таблицы. Что и почему произошло? как проигнорировать ошибку и продолжить работу?


---
**СПРАВОЧНО**

Подготовка контейнера перед каждым тестом:
1. `docker stop pg_lesson9; docker rm pg_lesson9`
2. Редактируем конфиг файл `my-postgres.conf`
3. `docker run -d --name pg_lesson8 -v "$PWD/my-postgres.conf":/etc/postgresql/postgresql.conf -e POSTGRES_PASSWORD=postgres -p 6432:5432 postgres:14 -c "config_file=/etc/postgresql/postgresql.conf"`

Запуск каждого теста:
1. `pgbench -i -h localhost -p 6432 -U postgres postgres`
2. `pgbench -c 8 -P 60 -T 600 -h localhost -p 6432 -U postgres postgres`


---
**КОМАНДЫ**
|Описание|Команда|
|-|-|
|Позиция вставки LSN |SELECT pg_current_wal_insert_lsn();|
|Определить файл по LSN|SELECT pg_walfile_name('0/196D76F0');|
|Определить файл+смещение по LSN|SELECT file_name, upper(to_hex(file_offset)) file_offset FROM pg_walfile_name_offset('0/331E4E64');|
|Вывести список wal-файлов|SELECT * FROM pg_ls_waldir() ORDER BY modification;|
|Статистика+ bgwriter|SELECT * FROM pg_stat_bgwriter;|
|Настройки checkpoint|SELECT name, setting, unit, short_desc FROM pg_settings WHERE name like '%checkpoint%';|
|Настройки WAL|SELECT name, setting, unit, short_desc FROM pg_settings WHERE name like '%wal%';|
|Настройки логирования|SELECT name, setting, unit, short_desc FROM pg_settings WHERE name IN ('data_directory','logging_collector','log_directory','log_filename');|
|Вывести LSN в заголовке страницы|SELECT lsn FROM page_header(get_raw_page('test_text',0));|
|Получение дельты м/у двух LSN (байт)|SELECT '0/1672DB8'::pg_lsn - '0/1670928'::pg_lsn;|
|Дамп содержимого журнала|pg_waldump -p . -s 0/1670928 -e 0/1672DB8 000000010000000000000001|
|вывести управляющую информацию кластера|pg_controldata|

---
**ССЫЛКИ**
|Описание|Ссылка|
|-|-|
|Как найти лог|https://www.endpointdev.com/blog/2014/11/dear-postgresql-where-are-my-logs/|
|Статья про WAL|https://habr.com/ru/company/postgrespro/blog/459250/|


