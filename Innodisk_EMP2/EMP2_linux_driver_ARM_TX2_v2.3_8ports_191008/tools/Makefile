build:
	gcc regctl.c -o emp2reg

install: emp2reg
	# clean old stuff in /sbin
	if [ -f /sbin/emp2reg ]; then \
		rm -f /sbin/emp2*; \
	fi
	# install script and application 
	install -o 755 emp2cfg /usr/sbin/
	install -o 755 emp2init /usr/sbin/
	install -o 755 -s emp2reg /usr/sbin/
	# install common definition items in /usr/lib/emp2
	mkdir -p /usr/lib/emp2/
	install -o 644 emp2lib /usr/lib/emp2/

clean:
	rm -f emp2reg
	rm -f /sbin/emp2*
	rm -f /usr/sbin/emp2*
	rm -rf /usr/lib/emp2