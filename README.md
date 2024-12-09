# Superscalar Out-of-Order Pipeline with In-Order Commit

![image](https://github.com/user-attachments/assets/63832daa-7bbf-4fa9-903f-38e8d842791d)


A seven-stage out-of-order pipeline is shown in the Figure above. It consists of Fetch, Decode, Issue, Schedule, Execute, Writeback, and Commit stages. The newly added units include the Register Alias Table (RAT) and the ReOrder Buffer (ROB). 

* Create the functionality for the two newly added units RAT and ROB.
* For a 1-wide machine, integrate the created objects in your pipeline and populate the four functions of the pipeline: issue(), schedule(), writeback(), and commit(). Implementing two scheduling policies: in-order and out-of-order (oldest ready first).
* Extend the machine to be 2-wide superscalar.
   
