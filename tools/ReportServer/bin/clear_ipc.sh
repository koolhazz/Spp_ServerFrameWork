SHM_IDS=`ipcs -m | awk '{if (match($1, /0x008002/)) print $1}'`
for SHM_ID in $SHM_IDS 
do
#	echo "ipcrm -M $SHM_ID"
	ipcrm -M $SHM_ID 
done

SEM_IDS=`ipcs -s | awk '{if (match($1, /0x00800000/)) print $1}'`
for SEM_ID in $SEM_IDS
do
#	echo "ipcrm -S $SEM_ID"
	ipcrm -S $SEM_ID
done
