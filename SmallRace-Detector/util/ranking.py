import os
import sys


def twoForkAt(race):
    cs = race.split("---Callstacks---")[-1]
    # print(race)
    t1,t2 = cs.split("====")
    t1 = t1.lower()
    t2 = t2.lower()
    fork_names = ["fork","forkAt"]
    v1 = 0
    v2 = 0
    for fn in fork_names:
        v1 += t1.count(fn)
        v2 += t2.count(fn)
    if v1>1 and v2>1:
        return 20
    if v1>1 or v2>1:
        return 10
    return 0


def onlyOneCritical(race):
    cs = race.split("---Callstacks---")[-1]
    t1,t2 = cs.split("====")
    t1 = t1.lower()
    t2 = t2.lower()
    # t1,t2 = race.split("Thread 2 (")
    critical_name = ["critical","lock"]
    v1 = False
    v2 = False
    for fn in critical_name:
        if fn in t1:
            v1 = True
        if fn in t2:
            v2 = True
    if v1 and not v2 or not v1 and v2:
        return 10
    return 0


def shouldBeRead(race):
    return 0
    writeCnt = 0
    if "Thread 1 (write):" in race: writeCnt += 1
    if "Thread 2 (write):" in race: writeCnt += 1
    invalid = False
    lines = race.split("\n")
    last_names = set()
    for l in lines:
        
        if l[:3]==">>>":
            if 'st.' in l:
                fname = l.split('st.')[1]
                if '$' in fname:
                    fname = fname.split('$')[0]
                if '[' in fname:
                    fname = fname.split('[')[0]
                fname = fname.strip()
                if fname.endswith(":") and fname[:-1] in last_names:
                    invalid = True
                else:
                    last_names.add(fname)
                    # return -20
                # print(fname)
        if l.startswith("Thread 2 ("):
            if invalid:
                writeCnt -= 1
                invalid = False
            if writeCnt <=0:
                return -20
    if invalid:
        writeCnt -= 1
        invalid = False
    if writeCnt <=0:
        return -20
    return 0

def keyWords(race):
    start = ["startup","setup","initializeFrom", "initialize"]
    end = ["clearall","shutdown", "closeDemo", "disable","StartUpComplete","terminate", "deactivate"]
    cs = race.split("---Callstacks---")[-1]
    t1,t2 = cs.split("====")
    t1 = t1.lower()
    t2 = t2.lower()
    # t1,t2 = race.split("Thread 2 (")
    oneHasOneNot = 0
    for it in start:
        if it in t1 and it not in t2 or it in t2 and it not in t1:
            oneHasOneNot += 1
    for it in end:
        if it in t1 and it not in t2 or it in t2 and it not in t1:
            oneHasOneNot += 1
    oneHasOneNot = min(3,oneHasOneNot)
    return -oneHasOneNot*2

def localVarRace(race):
    cs = race.split("---LLVM IR---")[-1].split("---Callstacks---")[0]
    ret = cs.count("@local_")
    if ret <2:
        return 0
    return -1000

def localVar(race):
    cs = race.split("---Source Code---")[-1].split("---LLVM IR---")[0]
    ret = cs.count('|')
    if ret <=2:
        return 0
    return -1000

    return (ret - 2) * -3

    # lines = race.split("\n")
    # last_names = set()
    # for l in lines:
    #     if l.count('|')>=2:
    #         s = l.split("|")[-2]
    #         s = s.split("|")[-1]
    #         print(s)


    return 0

def newVar(race):
    lines = race.split("\n")
    last_names = set()
    newcnt = 0
    for l in lines:
        if len(l)>2 and l[0]==">" and l[1]!=">":
            if "new" in l:
                newcnt += 1
    newcnt = min(3,newcnt)
    return -newcnt*2

def generateRanking(race):
    
    # print(race)
    priority = 0
    # Higher Priority
    priority += twoForkAt(race)
    priority += onlyOneCritical(race)

    #Lower priority
    priority += shouldBeRead(race)
    priority += keyWords(race)
    priority += localVar(race)
    priority += localVarRace(race)
    priority += newVar(race)
    
    return priority


def printReport(res):
    for r in res:
        print("==== Found a potential race with priority: ", r[0])
        print(r[1])
def saveReport(res, dir):
    lim = 10
    if len(res)>lim:
        res = res[:lim]
    with open(dir + "/ranked_report.txt",'w') as f:
        for i,r in enumerate(res):
            f.write("-------------Race NO. "+str(i+1))
            f.write("==== Found a potential race with priority: ")
            f.write(str(r[0]))
            f.write("\n")
            f.write(r[1]+"\n")

def saveReport100(res, dir):
    lim = 100
    if len(res)>lim:
        res = res[:lim]
    with open(dir + "/ranked_report_top100.txt",'w') as f:
        for i,r in enumerate(res):
            f.write("-------------Race NO. "+str(i+1))
            f.write("==== Found a potential race with priority: ")
            f.write(str(r[0]))
            f.write("\n")
            f.write(r[1]+"\n")

def handleReport(report,dir):
    _, data = report.split("===== Program Trace =====")
    pt, race = data.split("==== Races ====")
    with open(dir + "/ProgramTrace.txt", "w") as f:
        f.write("===== Program Trace =====\n")
        f.write(pt)
    races = race.split("========== Found a race between: ==========")
    res = []
    for r in races:
        if "call" not in r:
            continue
        rank = generateRanking(r)
        res.append([rank,r])
    res = sorted(res,key = lambda x: x[0],reverse=True)
    # printReport(res)
    saveReport(res, dir)
    saveReport100(res, dir)


if __name__ == "__main__":
    dir = sys.argv[1]
    # output = open(dir + "/RankedReport.txt", "w")
    with open(dir + "/nohup.out") as f:
        report = f.read()

    handleReport(report,dir)