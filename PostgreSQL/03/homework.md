# 03. Установка PostgreSQL

>сделать ВМ с Ubuntu 20.04 и поставить на нем Docker Engine или развернуть докер любым удобным способом
```diff
+Имеем VM VirtualBox с Ubuntu 22.04
```
```
vboxuser@Ubuntu22:~$ lsb_release -a
No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 22.04.1 LTS
Release:        22.04
Codename:       jammy
```
```diff
+Накатываем актуальную сборку Docker Engine
```
Ссылка на [мануал](https://www.digitalocean.com/community/tutorials/how-to-install-and-use-docker-on-ubuntu-20-04-ru) (или [мануал2](https://docs.docker.com/engine/install/ubuntu/))
```diff
+Имеем Docker 23.0.0
```
```
vboxuser@Ubuntu22:~$ docker version
Client: Docker Engine - Community
 Version:           23.0.0
 API version:       1.42
 Go version:        go1.19.5
 Git commit:        e92dd87
 Built:             Wed Feb  1 17:49:08 2023
 OS/Arch:           linux/amd64
 Context:           default
```

>сделать каталог /var/lib/postgres

>развернуть контейнер с PostgreSQL 14 смонтировав в него /var/lib/postgres

>развернуть контейнер с клиентом postgres


>подключится из контейнера с клиентом к контейнеру с сервером и сделать таблицу с парой строк


>подключится к контейнеру с сервером с ноутбука/компьютера вне инстансов ВМ


>удалить контейнер с сервером


>создать его заново


>подключится снова из контейнера с клиентом к контейнеру с сервером


>проверить, что данные остались на месте


