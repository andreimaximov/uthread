#!/usr/bin/env python3

import contextlib
import os
import random
import socket
import string
import subprocess
import time
import unittest


def pathToExample(name):
    build = os.getenv('MESON_BUILD_ROOT', os.getcwd())
    example = os.path.join(build, name)
    if not os.path.exists(example):
        raise RuntimeError('{} not found!'.format(name))
    return example


@contextlib.contextmanager
def runSubprocess(command):
    process = subprocess.Popen(
        command,
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdout=subprocess.PIPE,
        universal_newlines=True,
    )
    try:
        yield process
    finally:
        process.kill()
        process.wait()


def runAndCheck(command, stdin=None):
    with runSubprocess(command) as p:
        return p.communicate(stdin)[0]


def randomStr(n):
    return ''.join(random.choice(string.ascii_lowercase) for _ in range(n))


class SmokeTest(unittest.TestCase):
    '''High level smoke tests for the library.

    These should be run via ninja smoke from your meson build directory.
    '''

    def testHelloWorld(self):
        stdout = runAndCheck(pathToExample('helloworld'))
        self.assertEqual(stdout, 'Hello... World!\n')

    def testTcpEcho(self):
        with runSubprocess([pathToExample('tcpecho'), '--tasks', '128']):
            time.sleep(1)

            sockets = []
            try:
                for _ in range(128):
                    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    s.connect(('127.0.0.1', 8000))
                    message = bytes(randomStr(1024), 'ascii')
                    sockets.append((s, message))

                for s, message in sockets:
                    s.sendall(message)

                for s, message in sockets:
                    echo = s.recv(1025)
                    self.assertEqual(echo, message)
            finally:
                for s, _ in sockets:
                    s.close()


if __name__ == '__main__':
    unittest.main()
