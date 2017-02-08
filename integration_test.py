import httplib
from subprocess import Popen
from subprocess import check_output
import os
import signal
import filecmp
import re

HOSTNAME = "localhost"
PORT = 8080
CONFIGFILENAME = "SimpleConfig"

def testCasesGenerator(url, method, expected_body, case_number):
    connection = httplib.HTTPConnection(HOSTNAME, PORT)
    connection.request(method, url)
    response = connection.getresponse()
    body = response.read()
    if expected_body == body:
        print "Case{0:d} passed!".format(case_number)
    else:
        print "Case{0:d} failed!".format(case_number)
    connection.close()

def testCase1():
    testCasesGenerator("/echo", "GET", "GET /echo HTTP/1.1\r\nHost: localhost:8080\r\nAccept-Encoding: identity\r\n\r\n", 1)
    
def testCase2():
    testCasesGenerator("/static/b.txt", "GET", "<html><body><p>File not found, can't open.</p></body></html>", 2)
    
def testCase3():
    basePath = ""
    configFile = open(CONFIGFILENAME, 'r')
    for line in configFile:
        tokens = line.lstrip(' \t')
        tokens = tokens.rstrip('\r\n;')
        tokens = re.split(' ', tokens)
        if tokens[0] == "base-path" or tokens[0] == "root":
            basePath = tokens[1]
            break;
    configFile.close()
    testFile = open(basePath + "/b.txt", 'w')
    testFile.write("abcde")
    testFile.close()
    testCasesGenerator("/static/b.txt", "GET", "abcde", 3)
    os.remove(basePath + "/b.txt")

def main():
    
    Popen(["./webserver", "SimpleConfig"])
    pid = check_output(["pidof", "-s", "./webserver"])
    testCase1()
    testCase2()
    testCase3()
    
    os.kill(int(pid), signal.SIGKILL)
    
if __name__ == "__main__":
    try:
        main()
    except:
        pid = check_output(["pidof", "-s", "./webserver"])
        if not pid == "":
            os.kill(int(pid), signal.SIGKILL)
        raise

