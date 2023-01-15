FROM mysql:5.7

# 语言编码
ENV LANG=C.UTF-8

# 设置mysql root用户密码
ENV MYSQL_ROOT_PASSWORD 123456

# 配置mysql编码为utf8
COPY mysql.cnf /etc/mysql/conf.d/mysql.cnf

# 初始化时执行init.sql
COPY init.sql /docker-entrypoint-initdb.d/init.sql

# 配置编码为utf8
CMD ["mysqld", "--character-set-server=utf8", "--collation-server=utf8_unicode_ci"]
