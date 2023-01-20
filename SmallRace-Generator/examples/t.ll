; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@delay1 = internal constant [6 x i8] c"delay1"
@Array = internal constant [5 x i8] c"Array"
@local_1000000 = internal constant [7 x i8] c"1000000"
@"local_'2 race'" = internal constant [8 x i8] c"'2 race'"
@local_31 = internal constant [2 x i8] c"31"
@delay2 = internal constant [6 x i8] c"delay2"
@"local_'#2 race: '" = internal constant [11 x i8] c"'#2 race: '"
@"st.anonfun.12*st.anonfun.8*st.anonfun.7*st.startThread2*Race1.1" = internal constant [61 x i8] c"st.anonfun.12*st.anonfun.8*st.anonfun.7*st.startThread2*Race1"
@"st.anonfun.11*st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1.2" = internal constant [75 x i8] c"st.anonfun.11*st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1"
@"st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1.3" = internal constant [61 x i8] c"st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1"
@local_2 = internal constant [1 x i8] c"2"
@"st.anonfun.9*st.anonfun.8*st.anonfun.7*st.startThread2*Race1.4" = internal constant [60 x i8] c"st.anonfun.9*st.anonfun.8*st.anonfun.7*st.startThread2*Race1"
@"st.anonfun.8*st.anonfun.7*st.startThread2*Race1.5" = internal constant [47 x i8] c"st.anonfun.8*st.anonfun.7*st.startThread2*Race1"
@"st.anonfun.7*st.startThread2*Race1.6" = internal constant [34 x i8] c"st.anonfun.7*st.startThread2*Race1"
@"local_'1 race'" = internal constant [8 x i8] c"'1 race'"
@local_30 = internal constant [2 x i8] c"30"
@shared = internal constant [6 x i8] c"shared"
@max = internal constant [3 x i8] c"max"
@"local_'#1 race: '" = internal constant [11 x i8] c"'#1 race: '"
@Transcript = internal constant [10 x i8] c"Transcript"
@"st.anonfun.6*st.anonfun.2*st.anonfun.1*st.startThread1*Race1.7" = internal constant [60 x i8] c"st.anonfun.6*st.anonfun.2*st.anonfun.1*st.startThread1*Race1"
@"global_op_>" = internal constant [1 x i8] c">"
@each = internal constant [4 x i8] c"each"
@"global_op_+" = internal constant [1 x i8] c"+"
@"st.anonfun.5*st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1.8" = internal constant [73 x i8] c"st.anonfun.5*st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1"
@"global_op_=" = internal constant [1 x i8] c"="
@"st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1.9" = internal constant [60 x i8] c"st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1"
@index = internal constant [5 x i8] c"index"
@"st.anonfun.3*st.anonfun.2*st.anonfun.1*st.startThread1*Race1.10" = internal constant [60 x i8] c"st.anonfun.3*st.anonfun.2*st.anonfun.1*st.startThread1*Race1"
@local_1 = internal constant [1 x i8] c"1"
@local_0 = internal constant [1 x i8] c"0"
@"st.anonfun.2*st.anonfun.1*st.startThread1*Race1.11" = internal constant [47 x i8] c"st.anonfun.2*st.anonfun.1*st.startThread1*Race1"
@"st.anonfun.1*st.startThread1*Race1.12" = internal constant [34 x i8] c"st.anonfun.1*st.startThread1*Race1"
@local_race = internal constant [4 x i8] c"race"
@thread2 = internal constant [7 x i8] c"thread2"
@local_self = internal constant [4 x i8] c"self"
@thread1 = internal constant [7 x i8] c"thread1"
@local_super = internal constant [5 x i8] c"super"
@local_10 = internal constant [2 x i8] c"10"
@Delay = internal constant [5 x i8] c"Delay"
@Race1 = internal constant [5 x i8] c"Race1"
@local_race1 = internal constant [5 x i8] c"race1"

declare i8* @malloc(i64)

declare void @free(i8*)

declare i8* @"st.forMilliseconds:"(i8*, i8*)

declare i8* @st.model.newObject2(i8*, i8*)

declare i8* @st.model.blockParam5(i8*, i8*, i8*, i8*, i8*)

declare i8* @"st.forkAt:named:"(i8*, i8*, i8*)

declare i8* @st.repeat(i8*)

declare i8* @st.model.blockParam4(i8*, i8*, i8*, i8*)

declare i8* @"st.ifTrue:"(i8*, i8*)

declare i8* @"st.show:"(i8*, i8*)

declare i8* @st.cr(i8*)

declare i8* @"st.do:"(i8*, i8*)

declare i8* @"st.ifFalse:"(i8*, i8*)

declare i8* @st.model.blockParam2(i8*, i8*)

declare i8* @st.model.binaryop(i8*, i8*, i8*)

declare i8* @"st.to:do:"(i8*, i8*, i8*)

declare i8* @st.model.blockParam3(i8*, i8*, i8*)

declare i8* @"st.at:put:"(i8*, i8*, i8*)

declare i8* @st.startThread2(i8*)

declare i8* @st.startThread1(i8*)

declare i8* @st.stopThread2(i8*)

declare i8* @st.stopThread1(i8*)

declare i8* @st.terminate(i8*)

declare i8* @st.model.instVar(i8*)

declare i8* @st.stop(i8*)

declare i8* @st.wait(i8*)

declare i8* @"st.forSeconds:"(i8*, i8*)

declare i8* @st.start(i8*)

declare i8* @st.initialize(i8*)

declare void @st.model.opaqueAssign(i8*, i8*)

declare i8* @st.model.newObject(i8*)

declare i8* @st.model.newTemp(i8*)

define i64 @main() !dbg !3 {
  %1 = call i8* @st.model.newTemp(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @local_race1, i64 0, i64 0)), !dbg !7
  %2 = call i8* @st.model.newObject(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @Race1, i64 0, i64 0)), !dbg !9
  call void @st.model.opaqueAssign(i8* %1, i8* %2), !dbg !10
  %3 = call i8* @st.initialize(i8* %1), !dbg !11
  %4 = call i8* @st.start(i8* %1), !dbg !12
  %5 = call i8* @"st.forSeconds:"(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @Delay, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @local_10, i64 0, i64 0)), !dbg !13
  %6 = call i8* @st.wait(i8* %5), !dbg !14
  %7 = call i8* @st.stop(i8* %1), !dbg !15
  ret i64 0, !dbg !16
}

define i8* @"st.new$Race1 class"(i8* %0) !dbg !17 {
  %2 = call i8* @st.model.newObject(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @local_super, i64 0, i64 0)), !dbg !19
  %3 = call i8* @st.initialize(i8* %2), !dbg !19
  ret i8* %3, !dbg !21
}

define i8* @"st.stopThread1$Race1"(i8* %0) !dbg !22 {
  %2 = call i8* @st.model.instVar(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @thread1, i64 0, i64 0)), !dbg !23
  %3 = call i8* @st.terminate(i8* %2), !dbg !23
  ret i8* %0, !dbg !25
}

define i8* @"st.stop$Race1"(i8* %0) !dbg !26 {
  %2 = call i8* @st.stopThread1(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @local_self, i64 0, i64 0)), !dbg !27
  %3 = call i8* @st.stopThread2(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @local_self, i64 0, i64 0)), !dbg !29
  ret i8* %0, !dbg !30
}

define i8* @"st.stopThread2$Race1"(i8* %0) !dbg !31 {
  %2 = call i8* @st.model.instVar(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @thread2, i64 0, i64 0)), !dbg !32
  %3 = call i8* @st.terminate(i8* %2), !dbg !32
  ret i8* %0, !dbg !34
}

define i8* @"st.start$Race1"(i8* %0) !dbg !35 {
  %2 = call i8* @st.startThread1(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @local_self, i64 0, i64 0)), !dbg !36
  %3 = call i8* @st.startThread2(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @local_self, i64 0, i64 0)), !dbg !38
  ret i8* %0, !dbg !39
}

define i8* @"st.anonfun.1*st.startThread1*Race1"(i8* %0, i8* %1, i8* %2, i8* %3) !dbg !40 {
  %5 = call i8* @st.model.blockParam4(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @"st.anonfun.2*st.anonfun.1*st.startThread1*Race1.11", i64 0, i64 0), i8* %1, i8* %2, i8* %3), !dbg !41
  %6 = call i8* @st.repeat(i8* %5), !dbg !41
  ret i8* %0, !dbg !43
}

define i8* @"st.anonfun.2*st.anonfun.1*st.startThread1*Race1"(i8* %0, i8* %1, i8* %2, i8* %3) !dbg !44 {
  call void @st.model.opaqueAssign(i8* %2, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_0, i64 0, i64 0)), !dbg !45
  %5 = call i8* @st.model.blockParam3(i8* getelementptr inbounds ([60 x i8], [60 x i8]* @"st.anonfun.3*st.anonfun.2*st.anonfun.1*st.startThread1*Race1.10", i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @index, i64 0, i64 0), i8* %3), !dbg !47
  %6 = call i8* @"st.to:do:"(i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_1, i64 0, i64 0), i8* %1, i8* %5), !dbg !48
  %7 = call i8* @st.model.blockParam3(i8* getelementptr inbounds ([60 x i8], [60 x i8]* @"st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1.9", i64 0, i64 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @each, i64 0, i64 0), i8* %2), !dbg !49
  %8 = call i8* @"st.do:"(i8* %3, i8* %7), !dbg !50
  %9 = call i8* @st.model.binaryop(i8* getelementptr inbounds ([1 x i8], [1 x i8]* @"global_op_>", i64 0, i64 0), i8* %2, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_0, i64 0, i64 0)), !dbg !51
  %10 = call i8* @st.model.blockParam2(i8* getelementptr inbounds ([60 x i8], [60 x i8]* @"st.anonfun.6*st.anonfun.2*st.anonfun.1*st.startThread1*Race1.7", i64 0, i64 0), i8* %2), !dbg !52
  %11 = call i8* @"st.ifTrue:"(i8* %9, i8* %10), !dbg !51
  ret i8* %0, !dbg !53
}

define i8* @"st.anonfun.3*st.anonfun.2*st.anonfun.1*st.startThread1*Race1"(i8* %0, i8* %1, i8* %2) !dbg !54 {
  %4 = call i8* @"st.at:put:"(i8* %2, i8* %1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_1, i64 0, i64 0)), !dbg !55
  ret i8* %0, !dbg !57
}

define i8* @"st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1"(i8* %0, i8* %1, i8* %2) !dbg !58 {
  %4 = call i8* @st.model.binaryop(i8* getelementptr inbounds ([1 x i8], [1 x i8]* @"global_op_=", i64 0, i64 0), i8* %1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_1, i64 0, i64 0)), !dbg !59
  %5 = call i8* @st.model.blockParam2(i8* getelementptr inbounds ([73 x i8], [73 x i8]* @"st.anonfun.5*st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1.8", i64 0, i64 0), i8* %2), !dbg !61
  %6 = call i8* @"st.ifFalse:"(i8* %4, i8* %5), !dbg !59
  ret i8* %0, !dbg !62
}

define i8* @"st.anonfun.5*st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1"(i8* %0, i8* %1) !dbg !63 {
  %3 = call i8* @st.model.binaryop(i8* getelementptr inbounds ([1 x i8], [1 x i8]* @"global_op_+", i64 0, i64 0), i8* %1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_1, i64 0, i64 0)), !dbg !64
  call void @st.model.opaqueAssign(i8* %1, i8* %3), !dbg !66
  ret i8* %0, !dbg !67
}

define i8* @"st.anonfun.6*st.anonfun.2*st.anonfun.1*st.startThread1*Race1"(i8* %0, i8* %1) !dbg !68 {
  %3 = call i8* @st.cr(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @Transcript, i64 0, i64 0)), !dbg !69
  %4 = call i8* @"st.show:"(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @Transcript, i64 0, i64 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @"local_'#1 race: '", i64 0, i64 0)), !dbg !71
  %5 = call i8* @"st.show:"(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @Transcript, i64 0, i64 0), i8* %1), !dbg !72
  ret i8* %0, !dbg !73
}

define i8* @"st.startThread1$Race1"(i8* %0) !dbg !74 {
  %2 = call i8* @st.model.newTemp(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @local_race, i64 0, i64 0)), !dbg !75
  %3 = call i8* @st.model.instVar(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @max, i64 0, i64 0)), !dbg !77
  %4 = call i8* @st.model.instVar(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @shared, i64 0, i64 0)), !dbg !77
  %5 = call i8* @st.model.blockParam4(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @"st.anonfun.1*st.startThread1*Race1.12", i64 0, i64 0), i8* %3, i8* %2, i8* %4), !dbg !77
  %6 = call i8* @"st.forkAt:named:"(i8* %5, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @local_30, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @"local_'1 race'", i64 0, i64 0)), !dbg !77
  %7 = call i8* @st.model.instVar(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @thread1, i64 0, i64 0)), !dbg !78
  call void @st.model.opaqueAssign(i8* %7, i8* %6), !dbg !78
  ret i8* %0, !dbg !79
}

define i8* @"st.anonfun.7*st.startThread2*Race1"(i8* %0, i8* %1, i8* %2, i8* %3, i8* %4) !dbg !80 {
  %6 = call i8* @st.model.blockParam5(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @"st.anonfun.8*st.anonfun.7*st.startThread2*Race1.5", i64 0, i64 0), i8* %1, i8* %2, i8* %3, i8* %4), !dbg !81
  %7 = call i8* @st.repeat(i8* %6), !dbg !81
  ret i8* %0, !dbg !83
}

define i8* @"st.anonfun.8*st.anonfun.7*st.startThread2*Race1"(i8* %0, i8* %1, i8* %2, i8* %3, i8* %4) !dbg !84 {
  %6 = call i8* @st.wait(i8* %1), !dbg !85
  call void @st.model.opaqueAssign(i8* %3, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_0, i64 0, i64 0)), !dbg !87
  %7 = call i8* @st.model.blockParam3(i8* getelementptr inbounds ([60 x i8], [60 x i8]* @"st.anonfun.9*st.anonfun.8*st.anonfun.7*st.startThread2*Race1.4", i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @index, i64 0, i64 0), i8* %4), !dbg !88
  %8 = call i8* @"st.to:do:"(i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_1, i64 0, i64 0), i8* %2, i8* %7), !dbg !89
  %9 = call i8* @st.model.blockParam3(i8* getelementptr inbounds ([61 x i8], [61 x i8]* @"st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1.3", i64 0, i64 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @each, i64 0, i64 0), i8* %3), !dbg !90
  %10 = call i8* @"st.do:"(i8* %4, i8* %9), !dbg !91
  %11 = call i8* @st.model.binaryop(i8* getelementptr inbounds ([1 x i8], [1 x i8]* @"global_op_>", i64 0, i64 0), i8* %3, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_0, i64 0, i64 0)), !dbg !92
  %12 = call i8* @st.model.blockParam2(i8* getelementptr inbounds ([61 x i8], [61 x i8]* @"st.anonfun.12*st.anonfun.8*st.anonfun.7*st.startThread2*Race1.1", i64 0, i64 0), i8* %3), !dbg !93
  %13 = call i8* @"st.ifTrue:"(i8* %11, i8* %12), !dbg !92
  ret i8* %0, !dbg !94
}

define i8* @"st.anonfun.9*st.anonfun.8*st.anonfun.7*st.startThread2*Race1"(i8* %0, i8* %1, i8* %2) !dbg !95 {
  %4 = call i8* @"st.at:put:"(i8* %2, i8* %1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_2, i64 0, i64 0)), !dbg !96
  ret i8* %0, !dbg !98
}

define i8* @"st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1"(i8* %0, i8* %1, i8* %2) !dbg !99 {
  %4 = call i8* @st.model.binaryop(i8* getelementptr inbounds ([1 x i8], [1 x i8]* @"global_op_=", i64 0, i64 0), i8* %1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_2, i64 0, i64 0)), !dbg !100
  %5 = call i8* @st.model.blockParam2(i8* getelementptr inbounds ([75 x i8], [75 x i8]* @"st.anonfun.11*st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1.2", i64 0, i64 0), i8* %2), !dbg !102
  %6 = call i8* @"st.ifFalse:"(i8* %4, i8* %5), !dbg !100
  ret i8* %0, !dbg !103
}

define i8* @"st.anonfun.11*st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1"(i8* %0, i8* %1) !dbg !104 {
  %3 = call i8* @st.model.binaryop(i8* getelementptr inbounds ([1 x i8], [1 x i8]* @"global_op_+", i64 0, i64 0), i8* %1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @local_1, i64 0, i64 0)), !dbg !105
  call void @st.model.opaqueAssign(i8* %1, i8* %3), !dbg !107
  ret i8* %0, !dbg !108
}

define i8* @"st.anonfun.12*st.anonfun.8*st.anonfun.7*st.startThread2*Race1"(i8* %0, i8* %1) !dbg !109 {
  %3 = call i8* @st.cr(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @Transcript, i64 0, i64 0)), !dbg !110
  %4 = call i8* @"st.show:"(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @Transcript, i64 0, i64 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @"local_'#2 race: '", i64 0, i64 0)), !dbg !112
  %5 = call i8* @"st.show:"(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @Transcript, i64 0, i64 0), i8* %1), !dbg !113
  ret i8* %0, !dbg !114
}

define i8* @"st.startThread2$Race1"(i8* %0) !dbg !115 {
  %2 = call i8* @st.model.newTemp(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @local_race, i64 0, i64 0)), !dbg !116
  %3 = call i8* @st.model.instVar(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @delay2, i64 0, i64 0)), !dbg !118
  %4 = call i8* @st.model.instVar(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @max, i64 0, i64 0)), !dbg !118
  %5 = call i8* @st.model.instVar(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @shared, i64 0, i64 0)), !dbg !118
  %6 = call i8* @st.model.blockParam5(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @"st.anonfun.7*st.startThread2*Race1.6", i64 0, i64 0), i8* %3, i8* %4, i8* %2, i8* %5), !dbg !118
  %7 = call i8* @"st.forkAt:named:"(i8* %6, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @local_31, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @"local_'2 race'", i64 0, i64 0)), !dbg !118
  %8 = call i8* @st.model.instVar(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @thread2, i64 0, i64 0)), !dbg !119
  call void @st.model.opaqueAssign(i8* %8, i8* %7), !dbg !119
  ret i8* %0, !dbg !120
}

define i8* @"st.initialize$Race1"(i8* %0) !dbg !121 {
  %2 = call i8* @st.model.instVar(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @max, i64 0, i64 0)), !dbg !122
  call void @st.model.opaqueAssign(i8* %2, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @local_1000000, i64 0, i64 0)), !dbg !122
  %3 = call i8* @st.model.instVar(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @max, i64 0, i64 0)), !dbg !124
  %4 = call i8* @st.model.newObject2(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @Array, i64 0, i64 0), i8* %3), !dbg !125
  %5 = call i8* @st.model.instVar(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @shared, i64 0, i64 0)), !dbg !126
  call void @st.model.opaqueAssign(i8* %5, i8* %4), !dbg !126
  %6 = call i8* @"st.forMilliseconds:"(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @Delay, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @local_10, i64 0, i64 0)), !dbg !127
  %7 = call i8* @st.model.instVar(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @delay1, i64 0, i64 0)), !dbg !128
  call void @st.model.opaqueAssign(i8* %7, i8* %6), !dbg !128
  %8 = call i8* @"st.forMilliseconds:"(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @Delay, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @local_10, i64 0, i64 0)), !dbg !129
  %9 = call i8* @st.model.instVar(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @delay2, i64 0, i64 0)), !dbg !130
  call void @st.model.opaqueAssign(i8* %9, i8* %8), !dbg !130
  ret i8* getelementptr inbounds ([4 x i8], [4 x i8]* @local_self, i64 0, i64 0), !dbg !131
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "mlir", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "LLVMDialectModule", directory: "/")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DISubprogram(name: "main", linkageName: "main", scope: null, file: !4, line: 1, type: !5, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!4 = !DIFile(filename: "Smalltalk_Examples/3/ExampleWSB.ws", directory: "/home/ubuntu")
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 1, column: 2, scope: !8)
!8 = !DILexicalBlockFile(scope: !3, file: !4, discriminator: 0)
!9 = !DILocation(line: 3, column: 9, scope: !8)
!10 = !DILocation(line: 3, scope: !8)
!11 = !DILocation(line: 4, scope: !8)
!12 = !DILocation(line: 6, scope: !8)
!13 = !DILocation(line: 7, column: 1, scope: !8)
!14 = !DILocation(line: 7, scope: !8)
!15 = !DILocation(line: 8, scope: !8)
!16 = !DILocation(line: 1, scope: !8)
!17 = distinct !DISubprogram(name: "st.new$Race1 class", linkageName: "st.new$Race1 class", scope: null, file: !18, line: 27, type: !5, scopeLine: 27, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!18 = !DIFile(filename: "Smalltalk_Examples/3/SharedStateExamples.st", directory: "/home/ubuntu")
!19 = !DILocation(line: 30, column: 2, scope: !20)
!20 = !DILexicalBlockFile(scope: !17, file: !18, discriminator: 0)
!21 = !DILocation(line: 27, scope: !20)
!22 = distinct !DISubprogram(name: "st.stopThread1$Race1", linkageName: "st.stopThread1$Race1", scope: null, file: !18, line: 37, type: !5, scopeLine: 37, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!23 = !DILocation(line: 39, column: 1, scope: !24)
!24 = !DILexicalBlockFile(scope: !22, file: !18, discriminator: 0)
!25 = !DILocation(line: 37, scope: !24)
!26 = distinct !DISubprogram(name: "st.stop$Race1", linkageName: "st.stop$Race1", scope: null, file: !18, line: 42, type: !5, scopeLine: 42, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!27 = !DILocation(line: 44, scope: !28)
!28 = !DILexicalBlockFile(scope: !26, file: !18, discriminator: 0)
!29 = !DILocation(line: 45, scope: !28)
!30 = !DILocation(line: 42, scope: !28)
!31 = distinct !DISubprogram(name: "st.stopThread2$Race1", linkageName: "st.stopThread2$Race1", scope: null, file: !18, line: 47, type: !5, scopeLine: 47, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!32 = !DILocation(line: 49, column: 1, scope: !33)
!33 = !DILexicalBlockFile(scope: !31, file: !18, discriminator: 0)
!34 = !DILocation(line: 47, scope: !33)
!35 = distinct !DISubprogram(name: "st.start$Race1", linkageName: "st.start$Race1", scope: null, file: !18, line: 51, type: !5, scopeLine: 51, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!36 = !DILocation(line: 53, scope: !37)
!37 = !DILexicalBlockFile(scope: !35, file: !18, discriminator: 0)
!38 = !DILocation(line: 54, scope: !37)
!39 = !DILocation(line: 51, scope: !37)
!40 = distinct !DISubprogram(name: "st.anonfun.1*st.startThread1*Race1", linkageName: "st.anonfun.1*st.startThread1*Race1", scope: null, file: !18, line: 62, type: !5, scopeLine: 62, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!41 = !DILocation(line: 63, column: 1, scope: !42)
!42 = !DILexicalBlockFile(scope: !40, file: !18, discriminator: 0)
!43 = !DILocation(line: 62, column: 1, scope: !42)
!44 = distinct !DISubprogram(name: "st.anonfun.2*st.anonfun.1*st.startThread1*Race1", linkageName: "st.anonfun.2*st.anonfun.1*st.startThread1*Race1", scope: null, file: !18, line: 63, type: !5, scopeLine: 63, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!45 = !DILocation(line: 63, column: 2, scope: !46)
!46 = !DILexicalBlockFile(scope: !44, file: !18, discriminator: 0)
!47 = !DILocation(line: 64, column: 15, scope: !46)
!48 = !DILocation(line: 64, column: 1, scope: !46)
!49 = !DILocation(line: 65, column: 12, scope: !46)
!50 = !DILocation(line: 65, column: 1, scope: !46)
!51 = !DILocation(line: 66, column: 1, scope: !46)
!52 = !DILocation(line: 68, column: 3, scope: !46)
!53 = !DILocation(line: 63, column: 1, scope: !46)
!54 = distinct !DISubprogram(name: "st.anonfun.3*st.anonfun.2*st.anonfun.1*st.startThread1*Race1", linkageName: "st.anonfun.3*st.anonfun.2*st.anonfun.1*st.startThread1*Race1", scope: null, file: !18, line: 64, type: !5, scopeLine: 64, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!55 = !DILocation(line: 64, column: 25, scope: !56)
!56 = !DILexicalBlockFile(scope: !54, file: !18, discriminator: 0)
!57 = !DILocation(line: 64, column: 15, scope: !56)
!58 = distinct !DISubprogram(name: "st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1", linkageName: "st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1", scope: null, file: !18, line: 65, type: !5, scopeLine: 65, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!59 = !DILocation(line: 65, column: 21, scope: !60)
!60 = !DILexicalBlockFile(scope: !58, file: !18, discriminator: 0)
!61 = !DILocation(line: 65, column: 39, scope: !60)
!62 = !DILocation(line: 65, column: 12, scope: !60)
!63 = distinct !DISubprogram(name: "st.anonfun.5*st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1", linkageName: "st.anonfun.5*st.anonfun.4*st.anonfun.2*st.anonfun.1*st.startThread1*Race1", scope: null, file: !18, line: 65, type: !5, scopeLine: 65, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!64 = !DILocation(line: 65, column: 48, scope: !65)
!65 = !DILexicalBlockFile(scope: !63, file: !18, discriminator: 0)
!66 = !DILocation(line: 65, column: 40, scope: !65)
!67 = !DILocation(line: 65, column: 39, scope: !65)
!68 = distinct !DISubprogram(name: "st.anonfun.6*st.anonfun.2*st.anonfun.1*st.startThread1*Race1", linkageName: "st.anonfun.6*st.anonfun.2*st.anonfun.1*st.startThread1*Race1", scope: null, file: !18, line: 68, type: !5, scopeLine: 68, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!69 = !DILocation(line: 68, column: 4, scope: !70)
!70 = !DILexicalBlockFile(scope: !68, file: !18, discriminator: 0)
!71 = !DILocation(line: 70, column: 4, scope: !70)
!72 = !DILocation(line: 71, column: 4, scope: !70)
!73 = !DILocation(line: 68, column: 3, scope: !70)
!74 = distinct !DISubprogram(name: "st.startThread1$Race1", linkageName: "st.startThread1$Race1", scope: null, file: !18, line: 56, type: !5, scopeLine: 56, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!75 = !DILocation(line: 59, column: 3, scope: !76)
!76 = !DILexicalBlockFile(scope: !74, file: !18, discriminator: 0)
!77 = !DILocation(line: 62, column: 1, scope: !76)
!78 = !DILocation(line: 61, column: 1, scope: !76)
!79 = !DILocation(line: 56, scope: !76)
!80 = distinct !DISubprogram(name: "st.anonfun.7*st.startThread2*Race1", linkageName: "st.anonfun.7*st.startThread2*Race1", scope: null, file: !18, line: 82, type: !5, scopeLine: 82, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!81 = !DILocation(line: 83, column: 1, scope: !82)
!82 = !DILexicalBlockFile(scope: !80, file: !18, discriminator: 0)
!83 = !DILocation(line: 82, column: 1, scope: !82)
!84 = distinct !DISubprogram(name: "st.anonfun.8*st.anonfun.7*st.startThread2*Race1", linkageName: "st.anonfun.8*st.anonfun.7*st.startThread2*Race1", scope: null, file: !18, line: 83, type: !5, scopeLine: 83, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!85 = !DILocation(line: 83, column: 2, scope: !86)
!86 = !DILexicalBlockFile(scope: !84, file: !18, discriminator: 0)
!87 = !DILocation(line: 84, column: 1, scope: !86)
!88 = !DILocation(line: 85, column: 15, scope: !86)
!89 = !DILocation(line: 85, column: 1, scope: !86)
!90 = !DILocation(line: 86, column: 12, scope: !86)
!91 = !DILocation(line: 86, column: 1, scope: !86)
!92 = !DILocation(line: 87, column: 1, scope: !86)
!93 = !DILocation(line: 89, column: 3, scope: !86)
!94 = !DILocation(line: 83, column: 1, scope: !86)
!95 = distinct !DISubprogram(name: "st.anonfun.9*st.anonfun.8*st.anonfun.7*st.startThread2*Race1", linkageName: "st.anonfun.9*st.anonfun.8*st.anonfun.7*st.startThread2*Race1", scope: null, file: !18, line: 85, type: !5, scopeLine: 85, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!96 = !DILocation(line: 85, column: 25, scope: !97)
!97 = !DILexicalBlockFile(scope: !95, file: !18, discriminator: 0)
!98 = !DILocation(line: 85, column: 15, scope: !97)
!99 = distinct !DISubprogram(name: "st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1", linkageName: "st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1", scope: null, file: !18, line: 86, type: !5, scopeLine: 86, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!100 = !DILocation(line: 86, column: 21, scope: !101)
!101 = !DILexicalBlockFile(scope: !99, file: !18, discriminator: 0)
!102 = !DILocation(line: 86, column: 39, scope: !101)
!103 = !DILocation(line: 86, column: 12, scope: !101)
!104 = distinct !DISubprogram(name: "st.anonfun.11*st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1", linkageName: "st.anonfun.11*st.anonfun.10*st.anonfun.8*st.anonfun.7*st.startThread2*Race1", scope: null, file: !18, line: 86, type: !5, scopeLine: 86, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!105 = !DILocation(line: 86, column: 48, scope: !106)
!106 = !DILexicalBlockFile(scope: !104, file: !18, discriminator: 0)
!107 = !DILocation(line: 86, column: 40, scope: !106)
!108 = !DILocation(line: 86, column: 39, scope: !106)
!109 = distinct !DISubprogram(name: "st.anonfun.12*st.anonfun.8*st.anonfun.7*st.startThread2*Race1", linkageName: "st.anonfun.12*st.anonfun.8*st.anonfun.7*st.startThread2*Race1", scope: null, file: !18, line: 89, type: !5, scopeLine: 89, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!110 = !DILocation(line: 89, column: 4, scope: !111)
!111 = !DILexicalBlockFile(scope: !109, file: !18, discriminator: 0)
!112 = !DILocation(line: 91, column: 4, scope: !111)
!113 = !DILocation(line: 92, column: 4, scope: !111)
!114 = !DILocation(line: 89, column: 3, scope: !111)
!115 = distinct !DISubprogram(name: "st.startThread2$Race1", linkageName: "st.startThread2$Race1", scope: null, file: !18, line: 76, type: !5, scopeLine: 76, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!116 = !DILocation(line: 79, column: 3, scope: !117)
!117 = !DILexicalBlockFile(scope: !115, file: !18, discriminator: 0)
!118 = !DILocation(line: 82, column: 1, scope: !117)
!119 = !DILocation(line: 81, column: 1, scope: !117)
!120 = !DILocation(line: 76, scope: !117)
!121 = distinct !DISubprogram(name: "st.initialize$Race1", linkageName: "st.initialize$Race1", scope: null, file: !18, line: 101, type: !5, scopeLine: 101, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!122 = !DILocation(line: 104, column: 1, scope: !123)
!123 = !DILexicalBlockFile(scope: !121, file: !18, discriminator: 0)
!124 = !DILocation(line: 105, column: 22, scope: !123)
!125 = !DILocation(line: 105, column: 11, scope: !123)
!126 = !DILocation(line: 105, column: 1, scope: !123)
!127 = !DILocation(line: 106, column: 11, scope: !123)
!128 = !DILocation(line: 106, column: 1, scope: !123)
!129 = !DILocation(line: 107, column: 11, scope: !123)
!130 = !DILocation(line: 107, column: 1, scope: !123)
!131 = !DILocation(line: 101, scope: !123)
