#!/bin/bash

SECRET_NAME="kubectl-config"

if kubectl get secrets | grep -qw "$SECRET_NAME"; then
  echo "Secret $SECRET_NAME already exists"
else
  echo "Creating secret $SECRET_NAME"
  kubectl create secret generic "$SECRET_NAME" --from-file=config=/home/crownlabs/.kube/config
fi

set -x

key_size=100
size=(5)
key_size_int=$(echo $key_size | awk '{print int($1+0.5)}')
num_peers=(6 2 1)

tc_bool=0 #enable communication delay emulation
#kubectl apply -f ./pvc.yaml

for s in ${num_peers[@]}
do
kubectl apply -f ./prod-pvc.yaml
kubectl apply -f ./cons-pvc.yaml
if [ $tc_bool -eq 1 ]
then
  helm install my-etcd ./etcd8s-net/ --set replicaCount=$s
  sleep 300
  ./etcd8s-net/config-tc.sh #set delays
  sleep 5
else
  helm install my-etcd /home/crownlabs/etcd-k8s-experiment/etcd/etcd8s  --set replicaCount=$s
  sleep 300
fi

for s1 in ${size[@]}
do
    ./producer.sh $s1 $key_size
     sleep 10
    ./consumer.sh
     sleep 20
done

kubectl delete deployment ms-producer
kubectl delete deployment ms-consumer
helm uninstall my-etcd
sleep 10

kubectl delete pvc --all
kubectl delete pv --all
sleep 3

if [ $tc_bool -eq 0 ]
then
  mv ./logs/throughput_put_50rep.csv ./logs/throughput_put_50rep_${key_size_int}kiB_${s}nodes.csv
  mv ./logs/throughput_get_50rep.csv ./logs/throughput_get_50rep_${key_size_int}kiB_${s}nodes.csv
else
  mv ./logs/throughput_put_50rep.csv ./logs/throughput_put_50rep_delay_${key_size_int}kiB_${s}nodes.csv
  mv ./logs/throughput_get_50rep.csv ./logs/throughput_get_50rep_delay_${key_size_int}kiB_${s}nodes.csv
fi
done
