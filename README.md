# Gossip Membership Implementation
An implementation of the Gossip based membership protocol, made for the "<a href="https://www.coursera.org/learn/cloud-computing?specialization=cloud-computing">Cloud Computation Part 1</a>" course from coursera. 

Note for anyone trying to cheat on the assignement, this code isn't complete with the steps necessary to pass the grader, they have been omitted on purpose, however it will get you pretty close to what you want to do. With proof: 
<p align="center">
<img src="https://github.com/ImmortalBoi/Gossip-membership/assets/54212167/f0814a61-c956-4c69-9a99-f9951b3794b8"> 
</p>


# Usage

1. Run make to compile everything:
  ```bash
make
  ```
2. Running the tests, you can find them in the testcases folder, in this example we'll run:
  ```bash
./Application testcases/singlefailure.conf > output.log 
  ```
3. Open the output.log file to get the membership tables, logs for removal and failure and other events. Or, open the dbg.log file to see the message log between nodes.

# Extra information

In case you're curious about what the assignment is about, you can read more about it in the <a href="https://github.com/ImmortalBoi/Gossip-membership/blob/main/mp1_specifications.pdf">mp1_specifications.pdf</a>.
