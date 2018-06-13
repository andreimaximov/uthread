#!/usr/bin/env python3

import os
import subprocess
import unittest


class SubprocessError(Exception):
    def __init__(self, subprocess_err):
        self._subprocess_err = subprocess_err

    def __str__(self):
        stdout = self._subprocess_err.stdout
        stderr = self._subprocess_err.stderr
        return 'err: {}, stdout: {}, stderr: {}'.format(
            self._subprocess_err,
            str(stdout, 'ascii') if stdout is not None else '',
            str(stderr, 'ascii') if stderr is not None else '')


def example(name):
    build = os.getenv('MESON_BUILD_ROOT', os.getcwd())
    example = os.path.join(build, name)
    if not os.path.exists(example):
        raise RuntimeError('{} not found!'.format(name))
    return example


def check(command):
    try:
        out = subprocess.run(
            command,
            check=True,
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE,
            timeout=10).stdout
        return str(out, 'ascii')
    except subprocess.CalledProcessError as e:
        raise SubprocessError(e)


class SmokeTest(unittest.TestCase):
    '''High level smoke tests for the library.

    These should be run via ninja smoke from your meson build directory.
    '''

    def test_helloworld(self):
        stdout = check(example('helloworld'))
        self.assertEqual(stdout, 'Hello... World!\n')


if __name__ == '__main__':
    unittest.main()
