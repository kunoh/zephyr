
import sys
from typing import NoReturn
import colorama

#: Color used (when applicable) for printing with inf()
INF_COLOR = colorama.Fore.LIGHTGREEN_EX

#: Color used (when applicable) for printing with wrn()
WRN_COLOR = colorama.Fore.LIGHTYELLOW_EX

#: Color used (when applicable) for printing with err() and die()
ERR_COLOR = colorama.Fore.LIGHTRED_EX

def _reset_colors(file):
    print(colorama.Style.RESET_ALL, end='', file=file, flush=True)

def inf(*args):
    print(INF_COLOR, end='')
    print(*args)
    _reset_colors(sys.stdout)

def wrn(*args):
    print(WRN_COLOR, end='', file=sys.stderr)
    print('WARNING: ', end='', file=sys.stderr)
    print(*args, file=sys.stderr)
    _reset_colors(sys.stderr)

def err(*args):
    print(ERR_COLOR, end='', file=sys.stderr)
    print('FATAL ERROR: ', end='', file=sys.stderr)
    print(*args, file=sys.stderr)
    _reset_colors(sys.stderr)

def die(*args, exit_code=1) -> NoReturn:
    print(ERR_COLOR, end='', file=sys.stderr)
    print('FATAL ERROR: ', end='', file=sys.stderr)
    print(*args, file=sys.stderr)
    _reset_colors(sys.stderr)
    sys.exit(exit_code)
