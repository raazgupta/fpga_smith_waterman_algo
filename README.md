# fpga_smith_waterman_algo

- First start an EC2 c4.2xlarge instance and set it up with FPGA Developer AMI. Instructions here: https://github.com/Xilinx/SDAccel-Tutorials/blob/master/docs/aws-getting-started/CPP/STEP1.md#4-running-the-sdaccel-hello-world-example-on-aws-f1
- For remote desktop access you need to follow these instructions to setup DCV Viewer: https://forums.aws.amazon.com/ann.jspa?annID=6352
- Using SDAccel write the host and kernel code. Build for Software (SW) and test functionality. Check Iteration Interval and FPGA utitlization in Vivado.
- After software version is working as expected, start EC2 f1.2xlarge instance and also set it up with FPGA Developer AMI.
- Copy host and kernel files to f1 instance and use SDAccel to build and run in Software. Double check functionality. Make sure you have atleast 5GB of free disk space. Next build for System. 
- This creates the xclbin file in the System folder of the Project workspace. 
- If you do not have an S3 bucket to store the AWS binary files, create it using these instructions: <br/>
In AWS web console, create an S3 bucket and new folders for dcp and log files.<br/>
Then check instructions: https://github.com/aws/aws-fpga/blob/master/SDAccel/docs/Setup_AWS_CLI_and_S3_Bucket.md<br/>
The userid, password needs to taken from AWS Identity and Access Management (IAM) section. Create a new user with 'Administrator Access'. Create and save Access Key. Use this as userid, password. <br/>
- Create the AWS FPGA binary by running this command: 
source $AWS_FPGA_REPO_DIR/sdaccel_setup.sh<br/>
cd $AWS_FPGA_REPO_DIR/SDAccel/tools/<br/>
cp <from System directory>/xclbin_file_name.xclbin . <br/>
./create_sdaccel_afi.sh -xclbin=xclbin_file_name.xclbin -s3_bucket=bucket-name -s3_dcp_key=dcp-folder-name -s3_logs_key=logs-folder-name<br/>
- Find the AFI IDs by opening the file: timestamp_afi_id.txt
- Check when the Code is in 'avaiable' state by running: aws ec2 describe-fpga-images --fpga-image-ids FpgaImageId
- When it is available, to execute the application on fpga, from the System folder execute: <br/>
source $AWS_FPGA_REPO_DIR/sdaccel_setup.sh
sudo sh<br/>
source /opt/xilinx/xrt/setup.sh<br/>
  soruce 
./file.exe awsxclbinfile<br/>

- Here is more information about how to load AWS images on fpga or check/clear current fpga image: https://github.com/aws/aws-fpga/blob/master/hdk/README.md#iss
