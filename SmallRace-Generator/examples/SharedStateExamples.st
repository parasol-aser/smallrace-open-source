<?xml version="1.0"?>

<st-source>
<time-stamp>From VisualWorks®, 7.4 of December 5, 2005 on December 11, 2020 at 4:00:37 pm</time-stamp>
<!-- Package SharedStateExamples* -->


<class>
<name>Race1</name>
<environment>Smalltalk</environment>
<super>Core.Object</super>
<private>false</private>
<indexed-type>none</indexed-type>
<inst-vars>max shared delay1 delay2 thread1 thread2 </inst-vars>
<class-inst-vars></class-inst-vars>
<imports></imports>
<category>SharedStateExamples</category>
<attributes>
<package>SharedStateExamples</package>
</attributes>
</class>


<methods>
<class-id>Race1 class</class-id> <category>instance creation</category>

<body package="SharedStateExamples" selector="new">new
	"Answer a newly created and initialized instance."

	^super new initialize</body>
</methods>


<methods>
<class-id>Race1</class-id> <category>examples</category>

<body package="SharedStateExamples" selector="stopThread1">stopThread1
	"comment stating purpose of message"
	thread1 terminate.
	</body>

<body package="SharedStateExamples" selector="stop">stop
	"comment stating purpose of message"
self stopThread1.
self stopThread2.</body>

<body package="SharedStateExamples" selector="stopThread2">stopThread2
	"comment stating purpose of message"
	thread2 terminate.</body>

<body package="SharedStateExamples" selector="start">start
	"comment stating purpose of message"
self startThread1.
self startThread2.</body>

<body package="SharedStateExamples" selector="startThread1">startThread1
	"comment stating purpose of message"

	| race |
	
	thread1 :=
	[
	[race := 0.
	1 to: max do: [:index | shared at: index put: 1].
	shared do: [:each | each = 1 ifFalse: [race := race + 1]].
	race &gt; 0 
		ifTrue: 
			[Transcript
				cr;
				show: '#1 race: ';
				show: race]] 
			repeat] 
			forkAt: 30
			named: '1 race'</body>

<body package="SharedStateExamples" selector="startThread2">startThread2
	"comment stating purpose of message"

	| race |
	
	thread2 :=
	[
	[delay2 wait.
	race := 0.
	1 to: max do: [:index | shared at: index put: 2].
	shared do: [:each | each = 2 ifFalse: [race := race + 1]].
	race &gt; 0 
		ifTrue: 
			[Transcript
				cr;
				show: '#2 race: ';
				show: race]] 
			repeat] 
			forkAt: 31
			named: '2 race'.</body>
</methods>

<methods>
<class-id>Race1</class-id> <category>initialize-release</category>

<body package="SharedStateExamples" selector="initialize">initialize
	"Initialize a newly created instance. This method must answer the receiver."

	max := 1000000.
	shared := Array new: max.
	delay1 := Delay forMilliseconds: 10.
	delay2 := Delay forMilliseconds: 10.
	" *** Replace this comment with the appropriate initialization code *** "
	^self</body>
</methods>



</st-source>
