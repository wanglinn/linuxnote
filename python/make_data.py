#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import os, sys, random
import time, datetime

g_uint8_max = 0xff;
g_uint8_min = 0
g_uint16_max = 0xffff
g_uint16_min  = 0
g_uint32_max = 0xffffffff
g_uint32_min = 0
g_uint64_max = 0xffffffffffffffff
g_uint64_min = 0

g_int8_max = 0xff>>1
g_int8_min = 1 - 0xff>>1
g_int16_max = 0xffff>>1
g_int16_min = 1 - 0xffff>>1
g_int32_max =     0xffffffff>>1
g_int32_min = 1 - 0xffffffff>>1
g_int64_max =     0xffffffffffffffff>>1
g_int64_min = 1 - 0xffffffffffffffff>>1


# todo
g_float_max = g_int32_max
g_float_min = g_int32_min

# todo
g_double_max = g_int32_max
g_double_min = g_int32_min

CHARS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789'

class cl_make_data:

    def get_uint8_value(self):
        return random.randint(g_uint8_min, g_uint8_max)

    def get_int8_value(self):
        return random.randint(g_int8_min, g_int8_max)

    def get_uint16_value(self):
        return random.randint(g_uint16_min, g_uint16_max)

    def get_int16_value(self):
        return random.randint(g_int16_min, g_int16_max)

    def get_uint32_value(self):
        return random.randint(g_uint32_min, g_uint32_max)

    def get_int32_value(self):
        return random.randint(g_int32_min, g_int32_max) 

    def get_uint64_value(self):
        return random.randint(g_uint64_min, g_uint64_max)

    def get_int64_value(self):
        return random.randint(g_int64_min, g_int64_max)

    def get_float_value(sef):
        return random.uniform(g_float_min, g_float_max)

    def data_datetime_between(self, start_datetime, end_datetime, foramt='%Y-%m-%d %H:%M:%S'):
        startdatetime = datetime.datetime.strptime(start_datetime, '%Y-%m-%d %H:%M:%S')
        starttimestamp = time.mktime(startdatetime.timetuple())

        enddatetime = datetime.datetime.strptime(end_datetime, '%Y-%m-%d %H:%M:%S')
        endtimestamp = time.mktime(enddatetime.timetuple())

        timestamp = random.randint(starttimestamp, endtimestamp)
        ltime = time.localtime(timestamp)
        return time.strftime(foramt, ltime)

    def data_date_between(self, start_date, end_date, format='%Y-%m-%d'):
        start_date_time = '{0} 00:00:00'.format(start_date)
        end_date_time = '{0} 23:59:59'.format(end_date)
        random_datetime = self.data_datetime_between(start_date_time, end_date_time)
        random_date = datetime.datetime.strptime(random_datetime.split()[0], '%Y-%m-%d').date()
        return datetime.datetime.strftime(random_date, format)

    def get_date_value(self, start_date, end_date):
        return self.data_date_between(start_date, end_date,'%Y-%m-%d')

    def get_datetime_value(self, start_date, end_date):
        return self.data_datetime_between(start_date, end_date, '%Y-%m-%d %H:%M:%S')

    def get_varchar_n_value(num):
        res = ''
        for i in range(num):
            res += random.choice(CHARS)
        return res


obj1= cl_make_data()

line = 0
maxline = 1000
content1 = ''
fd = open('1.data', 'w')
while line<maxline:
    content1 = content1 + str(obj1.get_int32_value()) + ',' + str(obj1.get_datetime_value()) + '\n'
    line = line + 1
    if line %100 == 0:
        fd.write(content1)
        content1 = ''
if content1 != '':
    fd.write(content1)

fd.close()
