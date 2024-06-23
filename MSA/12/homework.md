# 12. Базовые сущности Кubernetes: ReplicaSet, Deployment, Service, Ingress

## Написать манифесты для деплоя в k8s для этого сервиса.

Манифесты должны описывать сущности: Deployment, Service, Ingress.<BR>
В Deployment могут быть указаны Liveness, Readiness пробы.<BR>
Количество реплик должно быть не меньше 2. Image контейнера должен быть указан с Dockerhub.<BR>
Хост в ингрессе должен быть arch.homework. В итоге после применения манифестов GET запрос на http://arch.homework/health должен отдавать {“status”: “OK”}.<BR>

## На выходе предоставить

0) ссылку на github c манифестами. Манифесты должны лежать в одной директории, так чтобы можно было их все применить одной командой `kubectl apply -f .`<BR>
1) url, по которому можно будет получить ответ от сервиса (либо тест в postmanе).<BR>

<BR>

**Запуск:**
Ссылка на манифесты: https://github.com/LikerPic/OTUS/tree/master/MSA/12/Manifest<BR>
Добавляем строку в hosts: 
```console
127.0.0.1 arch.homework
```

Применяем манифесты:
```console
kubectl apply -f .
```

Проверяем:
```console
curl http://arch.homework/health
{"status" : "OK"}
```

```diff
+Работает!
```

Алтернативный запуск сервиса без правки hosts:
```console
curl -H "Host: arch.homework" http://localhost/health
{"status" : "OK"}
```

## Задание со звездой
В Ingress-е должно быть правило, которое форвардит все запросы с `/otusapp/{student name}/*` на сервис с rewrite-ом пути. Где `{student name}` - это имя студента.<BR>
Например: `curl arch.homework/otusapp/aeugene/health` -> рерайт пути на `arch.homework/health` <BR>
<BR>

В манифест Ingress добавляем annotations:
```console
  annotations:
    nginx.ingress.kubernetes.io/use-regex: "true"
    nginx.ingress.kubernetes.io/rewrite-target: /health
```

И добавляем еще один path:
```console
      - path: /otusapp/LikerPic(/|$)(.*)
        pathType: ImplementationSpecific
        backend:
          service:
            name: health-service
            port:
              number: 8000
```

Проверяем:
```console
curl http://arch.homework/otusapp/LikerPic/anytest
{"status" : "OK"}
```

```diff
+Работает!
```

Смущает только то, что на самом деле это не редирект, а отдельный `path`, который просто ссылается на тот же самый сервис. Не уверен, что срабатывает именно редирект на /health <BR>
Если посмотреть ответный заголовок по запросу, то нет изменения Location:
```console
>curl -i http://arch.homework/otusapp/LikerPic/anytest
HTTP/1.1 200 OK
Date: Sun, 23 Jun 2024 16:07:15 GMT
Content-Type: application/json
Content-Length: 17
Connection: keep-alive

{"status" : "OK"}
```



Описание rewrite:<BR>
https://github.com/kubernetes/ingress-nginx/blob/main/docs/examples/rewrite/README.md

