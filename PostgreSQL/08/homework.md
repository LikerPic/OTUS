# **08. MVCC, vacuum и autovacuum**

## 1 создать GCE инстанс типа e2-medium и диском 10GB
```diff
+Имеем VM VirtualBox с Ubuntu 22.04. Диск 25GB. Доступно 9GB
```
```console
vboxuser@Ubuntu22:~$ df -h /
Filesystem      Size  Used Avail Use% Mounted on
/dev/sda3        24G   14G  9.0G  61% /

```
На VM выделено **3 CPU**:

![VM_CPU](VM_CPU.png)

```console
vboxuser@Ubuntu22:~$ lscpu
Architecture:            x86_64
  CPU op-mode(s):        32-bit, 64-bit
  Address sizes:         39 bits physical, 48 bits virtual
  Byte Order:            Little Endian
CPU(s):                  3
  On-line CPU(s) list:   0-2
Vendor ID:               GenuineIntel
  Model name:            Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
    CPU family:          6
    Model:               158
    Thread(s) per core:  1
    Core(s) per socket:  3
    Socket(s):           1
    Stepping:            10
    BogoMIPS:            4415.98
```


## установить на него PostgreSQL 14 с дефолтными настройками
Ниже будем использовать docker контейнер `postgres:14`

## применить параметры настройки PostgreSQL из прикрепленного к материалам занятия файла
Файл к занятию:
```
max_connections = 40
shared_buffers = 1GB
effective_cache_size = 3GB
maintenance_work_mem = 512MB
checkpoint_completion_target = 0.9
wal_buffers = 16MB
default_statistics_target = 500
random_page_cost = 4
effective_io_concurrency = 2
work_mem = 6553kB
min_wal_size = 4GB
max_wal_size = 16GB
```

Чтобы задавать настройки кластера будем использовать файл настроек `my-postgres.conf` снаружи контейнера и будем передавать его в качестве параметра.
Подготовим файл с настройками `my-postgres.conf`:
```console
docker run -i --rm postgres:14 cat /usr/share/postgresql/postgresql.conf.sample > my-postgres.conf
```
Запустим контейнер:
```console
docker run -d \
--name pg_lesson8 \
-v "$PWD/my-postgres.conf":/etc/postgresql/postgresql.conf \
-e POSTGRES_PASSWORD=postgres \
-p 6432:5432 \
postgres:14 \
-c "config_file=/etc/postgresql/postgresql.conf"
```
Проверим дефолтную конфигурацию:
```console
vboxuser@Ubuntu22:/media/sf_Upload$ psql -h localhost -p 6432 -U postgres
Password for user postgres:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
Type "help" for help.

postgres=# select name,setting,unit from pg_settings where name IN
('max_connections',
'shared_buffers',
'effective_cache_size',
'maintenance_work_mem',
'checkpoint_completion_target',
'wal_buffers',
'default_statistics_target',
'random_page_cost',
'effective_io_concurrency',
'work_mem',
'min_wal_size',
'max_wal_size');
             name             | setting | unit
------------------------------+---------+------
 checkpoint_completion_target | 0.9     |
 default_statistics_target    | 100     |
 effective_cache_size         | 524288  | 8kB
 effective_io_concurrency     | 1       |
 maintenance_work_mem         | 65536   | kB
 max_connections              | 100     |
 max_wal_size                 | 1024    | MB
 min_wal_size                 | 80      | MB
 random_page_cost             | 4       |
 shared_buffers               | 1024    | 8kB
 wal_buffers                  | 32      | 8kB
 work_mem                     | 4096    | kB
(12 rows)
```
Удалим контейнер.
```console
docker stop pg_lesson8; docker rm pg_lesson8
```

Отредактируем файл конфигурации `my-postgres.conf` - добавим параметры из файла к уроку.
Запускаем новый контейнер с новыми настройками.
Проверяем, что настройки применились:
```console
             name             | setting | unit
------------------------------+---------+------
 checkpoint_completion_target | 0.9     |
 default_statistics_target    | 500     |
 effective_cache_size         | 393216  | 8kB
 effective_io_concurrency     | 2       |
 maintenance_work_mem         | 524288  | kB
 max_connections              | 40      |
 max_wal_size                 | 16384   | MB
 min_wal_size                 | 4096    | MB
 random_page_cost             | 4       |
 shared_buffers               | 131072  | 8kB
 wal_buffers                  | 2048    | 8kB
 work_mem                     | 6553    | kB
```
Ок.


## выполнить pgbench -i postgres
`pgbench` — это простая программа для запуска тестов производительности Postgres Pro. Она многократно выполняет одну последовательность команд, возможно в параллельных сеансах базы данных, а затем вычисляет среднюю скорость транзакций (число транзакций в секунду). По умолчанию pgbench тестирует сценарий, примерно соответствующий TPC-B, который состоит из пяти команд SELECT, UPDATE и INSERT в одной транзакции. Однако вы можете легко протестировать и другие сценарии, написав собственные скрипты транзакций.
Точный состав транзакции указан в описании `pgbench` по ссылке в конце дока.

## запустить pgbench -c8 -P 60 -T 600 -U postgres postgres
## дать отработать до конца
## дальше настроить autovacuum максимально эффективно
## построить график по получившимся значениям
## так чтобы получить максимально ровное значение tps

