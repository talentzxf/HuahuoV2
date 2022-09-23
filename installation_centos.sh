yum groupinstall 'Development Tools'
yum install vim git -y
yum install httpd -y
yum install tar -y
yum install bzip2  -y
yum install npm nodejs -y
npm install -g npm@latest
yum install wget -y
wget https://github.com/Kitware/CMake/releases/download/v3.24.0/cmake-3.24.0-linux-x86_64.sh
chmod +x ./cmake-3.24.0-linux-x86_64.sh
source ./cmake-3.24.0-linux-x86_64.sh # Finish the cmake installation manually
wget https://downloads.mariadb.com/MariaDB/mariadb_repo_setup
chmod +x mariadb_repo_setup
sudo ./mariadb_repo_setup
sudo yum install MariaDB-server MariaDB-client MariaDB-backup
sudo systemctl start mariadb.service

# cert

# https://www.techrepublic.com/article/how-to-enable-https-on-apache-centos/

sudo yum install mod_ssl openssl -y
sudo openssl genrsa -out ca.key 2048
sudo openssl req -new -key ca.key -out ca.csr
sudo openssl x509 -req -days 365 -in ca.csr -signkey ca.key -out ca.crt
sudo cp ca.crt /etc/pki/tls/certs
sudo cp ca.key /etc/pki/tls/private/ca.key
sudo cp ca.csr /etc/pki/tls/private/ca.csr

sudo systemctl restart httpd

# Create virtual host
sudo mkdir -p /var/www/html/adorkable
sudo mkdir -p /etc/httpd/sites-available
sudo mkdir -p /etc/httpd/sites-enabled

# Create a
sudo ln -s /etc/httpd/sites-available/adorkable.conf /etc/httpd/sites-enabled/adorkable.conf
