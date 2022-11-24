
#!/bin/bash

echo `pwd`

ok=1

while [ $ok -ne 0 ] 
do
	git push origin dev
	ok=$?
done
