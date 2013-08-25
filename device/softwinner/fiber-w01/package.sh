cd $PACKAGE
if [ "$1" = "-d" ]; then
	echo "--------debug version, have uart printf-------------"
	./pack -c sun6i -p android -b  fiber-w01 -d card0 
else
	echo "--------release version, donnot have uart printf-------------"
	./pack -c sun6i -p android -b  fiber-w01 -d uart0
fi
