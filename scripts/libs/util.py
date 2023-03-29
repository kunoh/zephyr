import subprocess

def run_cmd(cmd):
    process = subprocess.Popen([cmd],
                     stdout=subprocess.PIPE,
                     stderr=subprocess.PIPE,
                     shell=True)
    stdout, stderr = process.communicate()
    return stdout, stderr