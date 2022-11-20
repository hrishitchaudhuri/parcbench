import subprocess

OS_PATH='x86_64-linux-gnu/'
BIN_PATH='bin/' + OS_PATH

LOG_PATH='logs/'
RD_TEST='../freqs.json'

def get_run_o2c(parallelism, filesz, filename):
    LOG_NAME=LOG_PATH + f'{filesz}_{parallelism}_o2c.txt'

    proc = subprocess.run(["time", BIN_PATH + "bw_file_rd", "-P", str(parallelism), str(filesz), "open2close", filename], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    with open(LOG_NAME, "w") as logfile:
        logfile.write(proc.stdout.decode())



if __name__=='__main__':
    for fsz in range(10000000, 50000000, 10000000):
        for par in [2 ** i for i in range(8)]:
            get_run_o2c(par, fsz, RD_TEST)