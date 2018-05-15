#!/bin/bash

### IPv4 ###

echo 1.2.3.4/32
echo 255.255.255.255
echo X 1.2.3
echo X 1.2.3.4/33
echo X 255.256.255.255


### IPv6 ###

echo A:b:3:4:5:6:7:8

echo ::

echo A::
echo A:b:3:4:5:6:7::
echo A::8
echo ::b:3:4:5:6:7:8
echo ::8

echo A:b:3:4:5:6::8
echo A:b:3:4:5::7:8
echo A:b:3:4::6:7:8
echo A:b:3::5:6:7:8
echo A:b::4:5:6:7:8
echo A::3:4:5:6:7:8

echo A::7:8
echo A:b:3:4:5::8
echo A::6:7:8
echo A:b:3:4::8
echo A::5:6:7:8
echo A:b:3::8
echo A::4:5:6:7:8
echo A:b::8
echo A::8

echo A:b:3:4:5:6:7:8/64

echo ::255.255.255.255
echo ::ffff:255.255.255.255
echo ::ffff:0:255.255.255.255
echo 00A:db8:3:4::192.0.2.33
echo 64:ff9b::192.0.2.33

echo fe80::1%tun

echo X fe80:1%tun
echo X A:b::8/129
echo X :::


### MAC Addresses ###

echo 0A:23:45:67:89:AB
echo 0A23.4567.89AB
echo X 0A:23:45:67:89
echo X 0A23.4567.89.AB


### Password ###

echo Password
echo X Pass1word
