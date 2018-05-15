#!/bin/bash

# Stores the correct ones
yes=()
# Stores the incorrect ones
no=()

### IPv4 ###

yes+=("1.2.3.4/32")
yes+=("255.255.255.255")
no+=("X 1.2.3")
no+=("X 1.2.3.4/33")
no+=("X 255.256.255.255")


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

no+=("X fe80:1%tun")
no+=("X A:b::8/129")
no+=("X :::")


### MAC Addresses ###

yes+=("0A:23:45:67:89:AB")
yes+=("0A23.4567.89AB")
no+=("X 0A:23:45:67:89")
no+=("X 0A23.4567.89.AB")


### Password ###

yes+=("Password")
no+=("X Pass1word")


### Syslog date ###

yes+=("Dec 03 12:34:56")
yes+=("jan 03 12:34:56")
no+=("X Dec 03 12:334:56")
no+=("X abc 03 12:34:56")



### Print all yes then all no

for ((i=0;i<${#yes[@]};++i)); do
  echo "${yes[$i]}"
done

echo ""

for ((i=0;i<${#no[@]};++i)); do
  echo "${no[$i]}"
done
