alter session set "_ORACLE_SCRIPT"=true;
CREATE USER RIS1 IDENTIFIED BY RIS1;
CREATE USER RIS2 IDENTIFIED BY RIS2;

GRANT ALL PRIVILEGES TO RIS1;
GRANT ALL PRIVILEGES TO RIS2;

--to ris1
create table Tris1
(
  id int,
  CONSTRAINT ris_pk PRIMARY KEY (id)
);

--to ris2
create table Tris2(id int);

CREATE PUBLIC DATABASE LINK riscon1
  CONNECT TO RIS1 IDENTIFIED BY "RIS1" USING '//127.0.0.1/orcl';

CREATE PUBLIC DATABASE LINK riscon2
  CONNECT TO RIS2 IDENTIFIED BY "RIS2" USING '//127.0.0.1/orcl';

begin
  insert into RIS2.Tris2@riscon2 values(5);
  insert into RIS2.Tris2@riscon2 values(6);
  commit;
end;

begin
  update RIS2.Tris2@riscon2 set id = 6 where id = 5;
  commit;
end;

select * from RIS2.Tris2@riscon2;

begin
  insert into RIS2.Tris2@riscon2 values(7);
  insert into RIS2.Tris2@riscon2 values(8);
  insert into RIS2.Tris2@riscon2 values('ggg');
  commit;
end;

