# 08. MVCC, vacuum и autovacuum

## 1 создать GCE инстанс типа e2-medium и диском 10GB
```diff
+Имеем VM VirtualBox с Ubuntu 22.04. Диск 25GB. Доступно 9GB
```
```console
vboxuser@Ubuntu22:~$ df -h /
Filesystem      Size  Used Avail Use% Mounted on
/dev/sda3        24G   14G  9.0G  61% /

```

## установить на него PostgreSQL 14 с дефолтными настройками
Запустим новый контейнер на версии PG 14
```console
vboxuser@Ubuntu22:~$ docker run --name pg_lesson8 -e POSTGRES_PASSWORD=postgres -d -p 6432:5432 postgres:14
ee5ec5f975224a374f99ddeee8bf5c7dfbc62e94a4c0bb9d9828d1619e45160c
vboxuser@Ubuntu22:~$ docker ps
CONTAINER ID   IMAGE         COMMAND                  CREATED          STATUS          PORTS                                       NAMES
ee5ec5f97522   postgres:14   "docker-entrypoint.s…"   31 seconds ago   Up 29 seconds   0.0.0.0:6432->5432/tcp, :::6432->5432/tcp   pg_lesson8
vboxuser@Ubuntu22:~$ psql -h localhost -p 6432 -U postgres
Password for user postgres:
psql (15.1 (Ubuntu 15.1-1.pgdg22.04+1), server 14.6 (Debian 14.6-1.pgdg110+1))
```


## применить параметры настройки PostgreSQL из прикрепленного к материалам занятия файла
## выполнить pgbench -i postgres
## запустить pgbench -c8 -P 60 -T 600 -U postgres postgres
## дать отработать до конца
## дальше настроить autovacuum максимально эффективно
## построить график по получившимся значениям
## так чтобы получить максимально ровное значение tps

