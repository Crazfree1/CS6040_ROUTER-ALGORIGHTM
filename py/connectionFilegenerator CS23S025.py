import random

def request_generator(req_File,request_count,node_count,min_bw,max_bw):
    with open(req_File, 'w') as connection_request:
        connection_request.write(f"{request_count}\n")
        req_ctr = 0
        while req_ctr !=request_count:
            s_node = random.randint(0,node_count-1)
            d_node = random.randint(0,node_count-1)
            while(s_node == d_node):
                d_node = random.randint(0,node_count-1)
            
            minBW = round(random.uniform(0,int(max_bw/3)),1)
            maxBw = round(random.uniform(min_bw,max_bw),1)
            while(maxBw <= minBW):
                maxBw = round(random.uniform(min_bw,max_bw),1)
            
            avgBW = round(random.uniform(minBW,maxBw),1)
            connection_request.write(f"{s_node} {d_node} {minBW} {avgBW} {maxBw}\n")
            req_ctr = req_ctr+1


