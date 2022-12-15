import subprocess
import config

class ParallelReadWriteLogger:
    def __init__(self, bin_path=config.BIN_PATH, rd_test=config.RD_TEST, log_path=config.LOG_PATH, size_lb=config.SIZE_LB, size_ub=config.SIZE_UB, size_step=config.SIZE_STEP):
        self._bin = bin_path
        self._rfile = rd_test
        self._logdir = log_path

        self._sizelb = size_lb
        self._sizeub = size_ub
        self._sizestep = size_step

    def run_o2c(self, fsize, parallelism):
        LOGS = self._logdir + f'{self._rfile}_{parallelism}_o2c.txt'

        proc = subprocess.run(["time", self._bin + "bw_file_rd", "-P", str(parallelism), str(fsize), "open2close", self._rfile], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        with open(LOGS, 'w') as logfile:
            logfile.write(proc.stdout.decode())

    def run_io(self, fsize, parallelism):
        LOGS = self._logdir + f'{self._rfile}_{parallelism}_o2c.txt'

        proc = subprocess.run(["time", self._bin + "bw_file_rd", "-P", str(parallelism), str(fsize), "io_only", self._rfile], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        with open(LOGS, 'w') as logfile:
            logfile.write(proc.stdout.decode())

    def start(self, parallelism, o2c=True):
        for fsize in range(self._sizelb, self._sizeub, self._sizestep):
            for pll in [2 ** i for i in range(parallelism)]:
                if o2c:
                    self.run_o2c(fsize, pll)

                else:
                    self.run_io(fsize, pll)


class ParallelFileSystemLogger:
    def __init__(self, bin_path=config.BIN_PATH, log_path=config.LOG_PATH, size_lb=config.SIZE_LB, size_ub=config.SIZE_UB, size_step=config.SIZE_STEP):
        self._bin = bin_path
        self._logdir = log_path

        self._sizelb = size_lb
        self._sizeub = size_ub
        self._sizestep = size_step

    def run_pfs(self, parallelism):
        LOGS = self._logdir + f'{parallelism}_pfs.txt'

        proc = subprocess.run(["time", self._bin + "lat_fs", "-P", str(parallelism)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        with open(LOGS, 'w') as logfile:
            logfile.write(proc.stdout.decode())

    def start(self, parallelism):
        for fsize in range(self._sizelb, self._sizeub, self._sizestep):
            for pll in [2 ** i for i in range(parallelism)]:
                self.run_pfs(parallelism)