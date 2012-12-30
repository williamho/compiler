int x;

main() {
	x = 50;
	printf("global variable x is %d\n\n",x);
	pointers();
	printf("after leaving function, global variable x is still %d\n\n",x);
	fibonacci();
	continues_and_breaks();
	printf("the function returnval() returned %d\n\n",x=returnval());
	dec2bin();
}

pointers() {
	int x, *p;
	x = 0;
	printf("in function, local var x is %d\n",x);

	printf("changing values via pointer indirection\n");
	p = &x;
	*p = 1234;
	printf("in function, local var x is now %d\n\n",x);
}

fibonacci() {
	int arr[15], i;
	arr[0] = arr[1] = 1;
	printf("fibonacci using a while-loop:\n%d %d ",*arr,*(arr+1));
	i = 2;
	while(i<15) {
		arr[i] = arr[i-1] + arr[i-2];
		printf("%d ",arr[i++]);
	}
	printf("\n\n");
}

continues_and_breaks() {
	int i;
	printf("example of continues and breaks in for-loop:\n");
	for (i=16; i>=0; i--) {
		if (i>8 && i%2) {
			printf(". ");
			continue;
		}
		if (i<3)
			break;
		printf("%d ",i);
	}
	printf("\n\n");
}

returnval() {
	int retval;
	printf("this function returns the value %d\n",retval = 21475);
	return retval;
}

dec2bin() {
	int i, flag;
	printf("left/right shift example\n");
	printf("%d in binary is: ",x);
	for (i=1<<16; i>0; i>>=1) {
		printf("%d",(i&x)!=0);
	}
	printf("\n\n");
}
