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
        process.communicate()


def runAndCheck(command, stdin=None):
    return subprocess.Popen(
        command,
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdout=subprocess.PIPE,
        universal_newlines=True,
    ).communicate()[0]


def randomStr(n):
    return ''.join(random.choice(string.ascii_lowercase) for _ in range(n))


class SmokeTest(unittest.TestCase):
    '''High level smoke tests for the library.

    These should be run via ninja smoke from your meson build directory.
    '''

    def testHelloWorld(self):
        stdout = runAndCheck(pathToExample('helloworld'))
        self.assertEqual(stdout, 'Hello... World!\n')

    def testStackOverflow(self):
        with runSubprocess(pathToExample('stackoverflow')) as process:
            self.assertNotEqual(process.wait(), 0)

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

    def testTcpChat(self):
        def readMessage(s):
            return [
                s for s in str(s.recv(1024), 'ascii').split('\n') if len(s) > 0
            ]

        def sendMessage(s, message):
            s.sendall(bytes(message + '\n', 'ascii'))
            time.sleep(0.1)
            return readMessage(s)

        with runSubprocess([pathToExample('tcpchat')]):
            time.sleep(1)

            socketX, socketY, messagesX, messagesY = None, None, [], []

            try:
                socketX = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                socketY = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                socketX.connect(('127.0.0.1', 8000))
                time.sleep(0.1)
                socketY.connect(('127.0.0.1', 8000))
                time.sleep(0.1)

                messagesX += sendMessage(socketX, '0')
                messagesX += sendMessage(socketX, '1')
                messagesY += sendMessage(socketY, '2')
                messagesX += sendMessage(socketX, '3')
                messagesX += sendMessage(socketX, '4')
                messagesX += sendMessage(socketX, '5')
                messagesY += sendMessage(socketY, '6')
                messagesX += sendMessage(socketX, '7')
                messagesY += sendMessage(socketY, '8')
                messagesY += sendMessage(socketY, '9')

                messagesX += readMessage(socketX)
                socketX.close()
                socketX = None
                time.sleep(0.1)
                messagesY += readMessage(socketY)

            finally:
                if socketX is not None:
                    socketX.close()
                if socketY is not None:
                    socketY.close()

            messages = [
                'Anonymous Iguana: <Joined>',
                'Anonymous Turtle: <Joined>',
                'Anonymous Iguana: 0',
                'Anonymous Iguana: 1',
                'Anonymous Turtle: 2',
                'Anonymous Iguana: 3',
                'Anonymous Iguana: 4',
                'Anonymous Iguana: 5',
                'Anonymous Turtle: 6',
                'Anonymous Iguana: 7',
                'Anonymous Turtle: 8',
                'Anonymous Turtle: 9',
                'Anonymous Iguana: <Left>',
            ]
            self.assertEqual(messagesX, messages[:-1])
            self.assertEqual(messagesY, messages[1:])


if __name__ == '__main__':
    unittest.main()
