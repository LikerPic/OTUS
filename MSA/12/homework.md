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


