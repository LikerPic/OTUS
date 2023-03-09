
# **09. Журналы**
```diff
+Имеем VM VirtualBox с Ubuntu 22.04 + Docker
```

## 1. Настройте выполнение контрольной точки раз в 30 секунд.

Получим текущие настройки **CHECKPOINT**:
```console
SELECT name, setting, unit, short_desc FROM pg_settings WHERE name like '%checkpoint%';
             name             | setting | unit |                                        short_desc
------------------------------+---------+------+------------------------------------------------------------------------------------------
 checkpoint_completion_target | 0.9     |      | Отношение продолжительности сброса "грязных" буферов во время контрольной точки к интервалу контрольных точек.
 checkpoint_flush_after       | 32      | 8kB  | Число страниц, по достижении которого ранее выполненные операции записи сбрасываются на диск.
 checkpoint_timeout           | 300     | s    | Задаёт максимальное время между автоматическими контрольными точками WAL.
 checkpoint_warning           | 30      | s    | Задаёт максимальный интервал, в котором выдаётся предупреждение о том, что контрольные точки, вызванные активностью WAL, происходят слишком часто.
 log_checkpoints              | off     |      | Протоколировать каждую контрольную точку.
(5 rows)
```
Из интересного здесь:
- `checkpoint_timeout` - периодичность срабатывания КТ (с);
- `checkpoint_completion_target` - доля, применяемая к величине `checkpoint_timeout`. Задает время работы КТ (нужно для того, чтобы уменьшить нажим сброса грязных страниц);
- `log_checkpoints` - позволяет включать логирование каждой КТ. Нам в тесте это очень пригодится.

В конфиг-файле `my-postgres.conf` выставим:
- `checkpoint_timeout` = 30s;
- `log_checkpoints` = on.

---
Определим статус **ЛОГИРОВАНИЯ**:

Чтобы найти лог-файл PG, нужно обратиться в `$PGDATA` / `log_directory` (считаем, что путь указан относительный). При этом в контейнере почему-то выключено логирование `logging_collector`:
```console
postgres=# SELECT name, setting, unit, short_desc FROM pg_settings WHERE name IN ('data_directory','logging_collector','log_directory','log_filename');
       name        |            setting             | unit |                                 short_desc
-------------------+--------------------------------+------+----------------------------------------------------------------------------
 data_directory    | /var/lib/postgresql/data       |      | Sets the server's data directory.
 log_directory     | log                            |      | Sets the destination directory for log files.
 log_filename      | postgresql-%Y-%m-%d_%H%M%S.log |      | Sets the file name pattern for log files.
 logging_collector | off                            |      | Start a subprocess to capture stderr output and/or csvlogs into log files.
(4 rows)
```
Зная, что потребуется собрать информацию по отработавшим контрольным точкам, позабоитмся заранее о включении логирования.
В моем случае лог будем искать здесь: `/var/lib/postgresql/data/log`.
В конфиг-файле `my-postgres.conf` выставим:
- `logging_collector` = on
---

Определим настройки **WAL**:
```console
postgres=# SELECT name, setting, unit, short_desc FROM pg_settings WHERE name like '%wal%';
             name              |  setting  | unit |                                                 short_desc
-------------------------------+-----------+------+------------------------------------------------------------------------------------------------------------
 max_slot_wal_keep_size        | -1        | MB   | Задаёт максимальный размер WAL, который могут резервировать слоты репликации.
 max_wal_senders               | 10        |      | Задаёт предельное число одновременно работающих процессов передачи WAL.
 max_wal_size                  | 1024      | MB   | Задаёт размер WAL, при котором инициируется контрольная точка.
 min_wal_size                  | 80        | MB   | Задаёт минимальный размер WAL при сжатии.
 track_wal_io_timing           | off       |      | Собирает статистику по времени активности ввода/вывода WAL.
 wal_block_size                | 8192      |      | Показывает размер блока в журнале WAL.
 wal_buffers                   | 512       | 8kB  | Задаёт число буферов дисковых страниц в разделяемой памяти для WAL.
 wal_compression               | off       |      | Сжимать данные записываемых в WAL полных страниц заданным методом.
 wal_consistency_checking      |           |      | Задаёт перечень менеджеров ресурсов WAL, для которых выполняются проверки целостности WAL.
 wal_init_zero                 | on        |      | Записывать нули в новые файлы WAL перед первым использованием.
 wal_keep_size                 | 0         | MB   | Определяет предельный объём файлов WAL, сохраняемых для резервных серверов.
 wal_level                     | replica   |      | Задаёт уровень информации, записываемой в WAL.
 wal_log_hints                 | off       |      | Запись полных страниц в WAL при первом изменении после контрольной точки, даже при некритическом изменении.
 wal_receiver_create_temp_slot | off       |      | Определяет, должен ли приёмник WAL создавать временный слот репликации, если не настроен постоянный слот.
 wal_receiver_status_interval  | 10        | s    | Задаёт максимальный интервал между отчётами о состоянии приёмника WAL, отправляемыми передающему серверу.
 wal_receiver_timeout          | 60000     | ms   | Задаёт предельное время ожидания для получения данных от передающего сервера.
 wal_recycle                   | on        |      | Перерабатывать файлы WAL, производя переименование.
 wal_retrieve_retry_interval   | 5000      | ms   | Задаёт время задержки перед повторной попыткой обращения к WAL после неудачи.
 wal_segment_size              | 16777216  | B    | Показывает размер сегментов журнала предзаписи.
 wal_sender_timeout            | 60000     | ms   | Задаёт предельное время ожидания репликации WAL.
 wal_skip_threshold            | 2048      | kB   | Размер нового файла, при достижении которого файл не пишется в WAL, а сбрасывается на диск.
 wal_sync_method               | fdatasync |      | Выбирает метод принудительной записи изменений в WAL на диск.
 wal_writer_delay              | 200       | ms   | Задержка между сбросом WAL в процессе, записывающем WAL.
 wal_writer_flush_after        | 128       | 8kB  | Объём WAL, обработанный пишущим WAL процессом, при котором инициируется сброс журнала на диск.
(24 rows)

postgres=# SELECT name, setting, unit, short_desc FROM pg_settings WHERE name IN ('fsync','synchronous_commit','commit_delay','commit_siblings');
        name        | setting | unit |                                     short_desc
--------------------+---------+------+-------------------------------------------------------------------------------------
 commit_delay       | 0       |      | Задаёт задержку в микросекундах между фиксированием транзакций и сбросом WAL на диск.
 commit_siblings    | 5       |      | Задаёт минимальное число одновременно открытых транзакций, которое требуется для применения commit_delay.
 fsync              | on      |      | Принудительная запись изменений на диск.
 synchronous_commit | on      |      | Задаёт уровень синхронизации текущей транзакции.
(4 rows)
```
Из интересного:
- `wal_keep_size` - позволяет хранить файлы WAL в заданном минимальной объеме (`min_wal_size` здесь не срабатывает почему-то)
- `fsync` - по сути включает/отключает синхронный режим записи журнла.

**ПУТЬ хранения WAL-файлов**:
- настройки для каталога нет;
- путь: `$PGDATA` / `pg_wal`;
- для вынесения WAL на отдельные диски нужно использовать симлинк;
- для получения списка файлов есть функция: `SELECT * FROM pg_ls_waldir() ORDER BY modification;`
- размер одного файла = 16 МБ по умолчанию;
- имя файла состоит из двух частей:
  -  старшие 8 шестнадцатеричных разрядов показывают номер ветви времени (она используется при восстановлении из резервной копии)
  -  остаток соответствует старшим разрядам LSN (а оставшиеся младшие разряды LSN показывают смещение) - см `pg_walfile_name_offset`.

Файлы хранятся в минимально необходимом объеме (для возможности восстановления с последней КТ) и перезаписываются по кругу. Файлы также требуются для репликации и архивации.

В конфиг-файле `my-postgres.conf` выставим:
- `wal_keep_size` = 200
---
Итого, кластер будем запускать с такими настройками `my-postgres.conf`:
```
checkpoint_timeout = 30s
wal_keep_size = 200
logging_collector = on
log_checkpoints = on
```

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


