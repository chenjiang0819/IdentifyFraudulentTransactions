import os
from pathlib import Path
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[1]
PROGRAM = os.environ.get('PROGRAM', str(ROOT / 'main'))

class FraudTests(unittest.TestCase):
    def run_program(self, text, valid=True):
        result = subprocess.run([PROGRAM], input=text, text=True, capture_output=True, timeout=5)
        self.assertNotIn('runtime error:', result.stderr)
        self.assertNotIn('AddressSanitizer', result.stderr)
        self.assertEqual(result.returncode == 0, valid, result.stderr)
        return result.stdout

    def test_original_fixtures(self):
        for i in (0, 1):
            self.assertEqual(self.run_program((ROOT / f'test{i}.txt').read_text()),
                             (ROOT / f'test{i}-output.txt').read_text())

    def test_unsorted_cards_and_per_card_day_reset(self):
        text = ('bbbbbbbb 100 100\naaaaaaaa 100 100\n%\n'
                '000000000001 aaaaaaaa 2020:04:01:00:00:00 90\n'
                '000000000002 bbbbbbbb 2020:04:01:00:00:00 90\n'
                '000000000003 aaaaaaaa 2020:04:02:00:00:00 10\n'
                '000000000004 bbbbbbbb 2020:04:02:00:00:00 20\n')
        out = self.run_program(text)
        self.assertEqual(out.count('IN_BOTH_LIMITS'), 4)

    def test_boundary_amounts_and_empty_transactions(self):
        self.run_program('aaaaaaaa 100 100\n%\n')
        out = self.run_program('aaaaaaaa 2147483647 2147483647\n%\n'
            '000000000001 aaaaaaaa 2020:02:29:00:00:00 2147483647\n'
            '000000000002 aaaaaaaa 2020:02:29:00:00:00 1')
        self.assertIn('OVER_DAILY_LIMIT', out)

    def test_invalid_inputs(self):
        card = 'aaaaaaaa 100 100\n%\n'
        bad = ['', 'aaaaaaaa 100 100\n', 'A'*1000+' 100 100\n%\n',
            'aaaaaaaa 10000000000000000000000 1\n%\n',
            'aaaaaaaa 100 100\naaaaaaaa 10 10\n%\n',
            card+'000000000001 bbbbbbbb 2020:01:01:00:00:00 1\n',
            card+'000000000001 aaaaaaaa 2020:02:30:00:00:00 1\n',
            card+'000000000001 aaaaaaaa 2020:01:01:00:00:00 -1\n',
            card+'000000000001 aaaaaaaa 2020:01:01:00:00:00 1\n'*2,
            ''.join(f'{i:08d} 1 1\n' for i in range(101))+'%\n']
        for text in bad:
            with self.subTest(length=len(text)):
                self.run_program(text, valid=False)

    def test_many_records_and_repeated_execution(self):
        text = 'aaaaaaaa 100000 100\n%\n' + ''.join(
            f'{i:012d} aaaaaaaa 2020:01:01:00:00:00 1\n' for i in range(1000))
        expected = self.run_program(text)
        self.assertEqual(expected.count('IN_BOTH_LIMITS'), 1000)
        self.assertEqual(self.run_program(text), expected)

if __name__ == '__main__':
    unittest.main()
