-- 创建数据库
create database red_paper_db;
use red_paper_db;

-- 创建红包表
create table red_paper(
  rp_id int auto_increment primary key comment '红包id',
  amount_money float not null comment '红包总金额',
  amount_number int not null comment '红包总份额'
);

-- 创建红包分配表
create table rp_allocation(
  rpa_id int auto_increment primary key comment '分配id',
  rp_id int not null comment '关联红包记录表的外键',
  money float not null comment '分配金额',
  allocation_time datetime default now() comment '分配时间',
  constraint `fk_per_alloc` foreign key(rp_id) references red_paper(rp_id) on delete cascade
);

