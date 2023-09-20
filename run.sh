docker stop grpc_demo
docker rm grpc_demo
docker run --name grpc_demo -itd   -v $(pwd):/home  -e LOCAL_USER_NAME=$USER -e LOCAL_USER_ID=`id -u $USER` zizdlp/gperf tail -f /etc/hosts
docker exec -it grpc_demo bash