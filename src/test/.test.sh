#!/bin/bash

# Stores the correct ones
yes=()
# Stores the incorrect ones
no=()



### IPv4 ###

yes+=("1.2.3.4/32")
yes+=("255.255.255.255")
no+=("1.2.3")
no+=("1.2.3.4/33")
no+=("255.256.255.255")


### IPv6 ###

yes+=("A:b:3:4:5:6:7:8")

yes+=("::")

yes+=("A::")
yes+=("A:b:3:4:5:6:7::")
yes+=("A::8")
yes+=("::b:3:4:5:6:7:8")
yes+=("::8")

yes+=("A:b:3:4:5:6::8")
yes+=("A:b:3:4:5::7:8")
yes+=("A:b:3:4::6:7:8")
yes+=("A:b:3::5:6:7:8")
yes+=("A:b::4:5:6:7:8")
yes+=("A::3:4:5:6:7:8")

yes+=("A::7:8")
yes+=("A:b:3:4:5::8")
yes+=("A::6:7:8")
yes+=("A:b:3:4::8")
yes+=("A::5:6:7:8")
yes+=("A:b:3::8")
yes+=("A::4:5:6:7:8")
yes+=("A:b::8")
yes+=("A::8")

yes+=("A:b:3:4:5:6:7:8/64")

yes+=("::255.255.255.255")
yes+=("::ffff:255.255.255.255")
yes+=("::ffff:0:255.255.255.255")
yes+=("00A:db8:3:4::192.0.2.33")
yes+=("64:ff9b::192.0.2.33")

yes+=("fe80::1%tun")

no+=("fe80:1%tun")
no+=("A:b::8/129")
no+=(":::")


### MAC Addresses ###

yes+=("0A:23:45:67:89:AB")
yes+=("0A23.4567.89AB")
no+=("0A:23:45:67:89")
no+=("0A23.4567.89.AB")


### Password ###

yes+=("Password")
no+=("Pass1word")


### Syslog date ###

yes+=("Dec 03 12:34:56")
yes+=("jan 03 12:34:56")
no+=("Dec 03 12:334:56")
no+=("abc 03 12:34:56")


### Compound tests ###

yes+=("::255.255.255.255 1.1.1.1")
yes+=("Dec 03 12:34:56 0A:23:45:67:89:AB 192.168.0.1/24 fe80::0:1:A:B/64 a:b:c:de:f::0A")
yes+=("a:b:c:de:f::0A 192.168.0.1/24 0A:23:45:67:89:AB fe80::0:1:A:B/64 Dec 03 12:34:56")
no+=("HEY 30 12:33:56 0A:23:4A5:67:89:AB 192.1618.0.1/24 fe80:x:0:1:A:B/ a:b:c:de000:f::0A")


### Print all yes then all no

for ((i=0;i<${#yes[@]};++i)); do
  echo "${yes[$i]}"
done

echo ""

for ((i=0;i<${#no[@]};++i)); do
  echo "X ${no[$i]}"
done
