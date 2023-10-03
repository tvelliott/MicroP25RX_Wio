ls -t *.c | xargs sed -i 's/\r//g'
ls -t *.cpp | xargs sed -i 's/\r//g'
ls -t *.h | xargs sed -i 's/\r//g'
ls -t *.ino | xargs sed -i 's/\r//g'
