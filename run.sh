docker stop release
docker rm release
docker run --name release -itd   -v /var/run/docker.sock:/var/run/docker.sock -v /Users/zz/gitlab:/home  -e LOCAL_USER_NAME=$USER -e LOCAL_USER_ID=`id -u $USER` harbor.tsingj.local/wangxili/gerf@sha256:309a9ad9a6d91da67329474ab60155da67a7964b1747f12b8fd6db38777dd5f6 tail -f /etc/hosts
docker exec -it release bash