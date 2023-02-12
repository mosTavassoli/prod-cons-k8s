#!/bin/bash

#pod_ips=$(kubectl get pods --selector app.kubernetes.io/instance=my-etcd -o json | egrep '\"podIP\"' | grep -E -o '\"([0-9]{1,3}[\\.]){3}[0-9]{1,3}\"')
#pod_find=$(kubectl get pods -o jsonpath='{.items[*].metadata.name}' | tr ' ' '\n' | grep ms-producer)
#len=$(echo $pod_find | wc -w)

#echo $pod_ips

#set -x

initConsumerYAML () {
cat << EOF > ./consumer.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: ms-consumer
spec:
  replicas: 1
  selector:
    matchLabels:
      app: ms-consumer
  template:
   metadata:
      labels:
        app: ms-consumer
   spec:
      containers:
      - name: ms-consumer
        image: mostafa2020/prod-cons:v24
        command:
          - sh
          - -c
          - |
            ./bin/consumer
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
          claimName: cons-pvc
      affinity:
        nodeAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
            nodeSelectorTerms:
            - matchExpressions:
              - key: kubernetes.io/hostname
                operator: In
                values:
                - worker-2
EOF
}

initConsumerYAML
kubectl apply -f consumer.yaml

sleep 10
pod_name=$(kubectl get pods | grep ms-consumer | awk '{print $1}')
isDone=$(kubectl logs $pod_name | grep -oP 'cons-done')
while [ -z "$isDone" ]
do
  sleep 5
  isDone=$(kubectl logs $pod_name | grep -oP 'cons-done')
done

kubectl cp -n default  $pod_name:/app/logs ./logs
sleep 10

# sleep 20

# pod_name=$(kubectl get pods | grep ms-consumer | awk '{print $1}')
# get_l_per_sec=$(kubectl logs $pod_name | grep -oP 'Get_L keys/sec: \K[0-9]+')
# echo "GET_L" $get_l_per_sec "keys/sec"

# get_s_per_sec=$(kubectl logs $pod_name | grep -oP 'Get_S keys/sec: \K[0-9]+')
# echo "GET_S" $get_s_per_sec "keys/sec"

