for i in `ls pass??.json` ; do
	cat $i
	../../test_fasterjson $i
	if [ $? -ne 0 ] ; then
		echo "*** check $i failed"
		exit 1
	fi
	echo
done
echo "*** check all ok"

