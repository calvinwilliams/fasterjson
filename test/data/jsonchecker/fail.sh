for i in `ls fail??.json` ; do
	cat $i
	../../test_fasterjson $i
	if [ $? -eq 0 ] ; then
		echo "*** check $i failed"
		exit 1
	fi
	echo
done
echo "*** check all ok"

