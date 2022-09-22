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
sudo systemctl start mariadb.service
