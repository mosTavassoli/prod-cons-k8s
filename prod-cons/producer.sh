#!/bin/bash

#pod_ips=$(kubectl get pods --selector app.kubernetes.io/instance=my-etcd -o json | egrep '\"podIP\"' | grep -E -o '\"([0-9]{1,3}[\\.]){3}[0-9]{1,3}\"')
#pod_find=$(kubectl get pods -o jsonpath='{.items[*].metadata.name}' | tr ' ' '\n' | grep ms-producer)
#len=$(echo $pod_find | wc -w)

#echo $pod_ips
size=$1
key_size=$2
#echo $size
#echo $key_size
#set -x

initProducerYAML () {
cat << EOF > ./producer.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: ms-producer
spec:
  replicas: 1
  selector:
    matchLabels:
      app: ms-producer
  template:
   metadata:
      labels:
        app: ms-producer
   spec:
      containers:
      - name: ms-producer
        image: mostafa2020/prod-cons:v25
        command:
          - sh
          - -c
          - |
            ./bin/producer $size $key_size
        volumeMounts:
        - name: kubectl-config-volume
          mountPath: /root/.kube
          readOnly: true
        - name: data-volume
          mountPath: /app/logs
      volumes:
      - name: kubectl-config-volume
        secret:
          secretName: kubectl-config
      - name: data-volume
        persistentVolumeClaim:
          claimName: prod-pvc
EOF
}

initProducerYAML
kubectl apply -f producer.yaml
#kubectl delete deployment ms-producer
sleep 10
pod_name=$(kubectl get pods | grep ms-producer | awk '{print $1}')
isDone=$(kubectl logs $pod_name | grep -oP 'prod-done')
while [ -z "$isDone" ]
do
  sleep 5
  isDone=$(kubectl logs $pod_name | grep -oP 'prod-done')
done

kubectl cp -n default  $pod_name:/app/logs ./logs


