n=`ps -ef|grep spp_report|grep -v grep|wc -l`
if [ $n -lt 1 ]
then	
./spp_report ../etc/spp_report.xml 
fi
