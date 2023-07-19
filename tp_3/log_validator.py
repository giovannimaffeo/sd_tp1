import os
def validate(file_path):
    f = open(file_path, "r")
    lines = f.readlines()
    requests = []
    grants = []
    releases = []

    for line in lines:
        if ("[R] Request" in line):
            requests.append(int(line.split("-")[1]))
            continue
        if ("[S] Grant" in line):
            if (len(grants) != len(releases)):
                print(line)
                print(len(grants))
                print(len(releases))
                raise Exception("Invalid log file: invalid grants and releases sequence")
            grants.append(int(line.split("-")[1]))
            continue
        if ("[R] Release" in line):
            if (len(releases) != len(grants) - 1):
                raise Exception("Invalid log file: invalid grants and releases sequence")
            releases.append(int(line.split("-")[1]))
            continue

    for i in range(len(requests)):
        if (requests[i] != grants[i] or grants[i] != releases[i]):
            raise Exception("Invalid log file: invalid grants and releases sequence")

    print(f"Log file was successfully validated for {file_path}")

if __name__ == "__main__":
    for file in os.listdir():
        if 'Server_Log' in file:
            validate(file)