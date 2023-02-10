#!/bin/bash

#pod_ips=$(kubectl get pods --selector app.kubernetes.io/instance=my-etcd -o json | egrep '\"podIP\"' | grep -E -o '\"([0-9]{1,3}[\\.]){3}[0-9]{1,3}\"')
#pod_find=$(kubectl get pods -o jsonpath='{.items[*].metadata.name}' | tr ' ' '\n' | grep ms-producer)
#len=$(echo $pod_find | wc -w)

#echo $pod_ips

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
        image: mostafa2020/producer:v1
        command:
          - sh
          - -c
          - |
            ./bin/producer -1
        volumeMounts:
        - name: kubectl-config-volume
          mountPath: /root/.kube
          readOnly: true
      volumes:
      - name: kubectl-config-volume
        secret:
          secretName: kubectl-config 
      affinity:
        nodeAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
            nodeSelectorTerms:
            - matchExpressions:
              - key: kubernetes.io/hostname
                operator: In
                values:
                - worker-1
EOF
}

initProducerYAML

kubectl apply -f producer.yaml
#kubectl delete deployment ms-producer
sleep 10
pod_name=$(kubectl get pods | grep ms-producer | awk '{print $1}')
keys_per_sec=$(kubectl logs $pod_name | grep -oP 'keys/sec: \K[0-9]+')
echo "PUT" $keys_per_sec "keys/sec"
