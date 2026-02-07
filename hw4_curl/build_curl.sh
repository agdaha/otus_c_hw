#git clone https://github.com/curl/curl.git

if [ -d curl/build ]; then
    rm -rf curl/build
fi


mkdir curl/build
cd curl/build

cmake .. \
  -DHTTP_ONLY=OFF \
  -DCURL_DISABLE_FTP=ON \
  -DCURL_DISABLE_FILE=ON \
  -DCURL_DISABLE_LDAP=ON \
  -DCURL_DISABLE_RTSP=ON \
  -DCURL_DISABLE_DICT=ON \
  -DCURL_DISABLE_TFTP=ON \
  -DCURL_DISABLE_GOPHER=ON \
  -DCURL_DISABLE_IMAP=ON \
  -DCURL_DISABLE_IPFS=ON \
  -DCURL_DISABLE_IPNS=ON \
  -DCURL_DISABLE_POP3=ON \
  -DCURL_DISABLE_SMTP=ON \
  -DCURL_DISABLE_SCP=ON \
  -DCURL_DISABLE_SFTP=ON \
  -DCURL_DISABLE_SMB=ON \
  -DCURL_DISABLE_MQTT=ON \
  -DCURL_DISABLE_TELNET=OFF \
  -DBUILD_TESTING=OFF \
  -DCMAKE_USE_OPENSSL=ON

make

cp src/curl ../../curl_
../../curl_ --version