'''chromaterm tests'''
import sys

# Simulate that stdout is natively a tty during tests
sys.stdout.isatty = lambda: True
