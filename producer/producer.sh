#!/bin/bash

pod_ips=$(kubectl get pods --selector app.kubernetes.io/instance=my-etcd -o json | egrep '\"podIP\"' | grep -E -o '\"([0-9]{1,3}[\\.]){3}[0-9]{1,3}\"')

echo $pod_ips

initProducerYAML () {
cat << EOF > ./producer.yaml
apiVersion: batch/v1
kind: Job
metadata:
  name:  ms-producer
spec:
  template:
    metadata:
      name: ms-producer
    spec:
      containers:
      - name: ms-producer
        image: mostafa2020/producer:v14
        command:
          - sh
          - -c
          - |
           apk add curl
           export version=$(curl -sL https://dl.k8s.io/release/stable.txt)
           curl -L --remote-name-all https://dl.k8s.io/$version/bin/linux/amd64/{kubectl}
           chmod +x kubectl
           mv kubectl /usr/local/bin/
           curl -L --remote-name https://github.com/etcd-io/etcd/releases/download/v3.5.1/etcd-v3.5.1-linux-amd64.tar.gz
           tar xzvf etcd-v3.5.1-linux-amd64.tar.gz
           mv etcd-v3.5.1-linux-amd64/etcdctl /usr/local/bin/
           ./bin/producer $(echo $pod_ips | awk '{print $1}')
      restartPolicy: Never
  backoffLimit: 0
EOF
}

initProducerYAML

kubectl apply -f producer.yaml
