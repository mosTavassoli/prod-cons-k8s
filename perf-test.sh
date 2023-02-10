#!/bin/bash

SECRET_NAME="kubectl-config"

if kubectl get secrets | grep -qw "$SECRET_NAME"; then
  echo "Secret $SECRET_NAME already exists"
else
  echo "Creating secret $SECRET_NAME"
  kubectl create secret generic "$SECRET_NAME" --from-file=config=/home/crownlabs/.kube/config
fi

./producer/producer.sh
sleep 10
./consumer/consumer.sh