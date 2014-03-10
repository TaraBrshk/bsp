import os
import IPython

from collections import defaultdict
import math
import numpy as np

import matplotlib
import matplotlib.pyplot as plt

attrs = ['sum_cov_trace','waypoint_distance_error','solve_time','initialization_time']

class Data:
	def __init__(self):
		self.data = []
		
	def add(self, d):
		self.data.append(d)
		
	@property
	def mean(self):
		if len(self.data) > 0:
			return sum(self.data)/float(len(self.data))
		return None
		
	@property
	def sd(self):
		m = self.mean
		if m is None:
			return None
			
		return np.std(np.array(self.data))

class File:
    attrs = ['sum_cov_trace','waypoint_distance_error','solve_time','initialization_time','total_time'] # 'failure'
    slam_types = ['slam-traj', 'slam-belief', 'slam-state', 'slam-control']
    
    def __init__(self, file_name, slam_type, file_time):
        self.file_name = file_name
        self.slam_type = slam_type
        self.file_time = file_time
        
        # self.example[example_number][attr]
        self.example = defaultdict(lambda: defaultdict(float))
        self.attr_vals = defaultdict(float)
        
        self.num_landmarks = 0
        self.num_examples = 0
        
        self.process()
        
    def process(self):
        f = open(self.file_name,'r')
        with open(self.file_name) as f:
            for line in f:
                if self.num_landmarks == 0:
                    # first line is the number of landmarks
                    self.num_landmarks = int(line)
                    continue
                
                attr, val = line.split(' ')
                if attr == File.attrs[0]:
                    self.num_examples += 1
                self.example[self.num_examples-1][attr] += float(val)
                
            for i in xrange(self.num_examples):
                self.example[i]['total_time'] = self.example[i]['solve_time'] + self.example[i]['initialization_time']
                
    def get(self, attr):
        return [self.example[i][attr] for i in xrange(self.num_examples)]
        
    def getAverage(self, attr):
    	d = Data()
    	for i in xrange(self.num_examples):
    		d.add(self.example[i][attr])
    		
    	return d.mean
    
    def getSd(self, attr):
    	d = Data()
    	for i in xrange(self.num_examples):
    		d.add(self.example[i][attr])
    		
    	return d.sd
    
    
    def printAverages(self):
        for attr in File.attrs:
            avg = sum([self.example[i][attr] for i in xrange(self.num_examples)])/float(self.num_examples)
            print(attr + ': ' + str(avg))
        print('')
            
    @staticmethod
    def printStatistics(files):
        landmark_numbers = sorted(list(set([f.num_landmarks for f in files])))
        
        for num_landmarks in landmark_numbers:
            files_l = [f for f in files if f.num_landmarks == num_landmarks]
            combined_num_examples = float(sum([f.num_examples for f in files_l]))
            if combined_num_examples > 0:
                print('Number of landmarks: ' + str(num_landmarks))
                for attr in File.attrs:
                    attr_vals = []
                    d = Data()
                    for f in files_l:
                    	for datapoint in f.get(attr):
                    		d.add(datapoint)
                    print(attr + ': ' + str(d.mean) + ' +- ' + str(d.sd))
                print('')
                
    @staticmethod
    def compareAttr(file0, file1, attr):
    	return file0.getAverage(attr) / file1.getAverage(attr)
    	
        
    # cost / slam-traj cost
    # slam-traj-speed / speed
    @staticmethod
    def compare(files0, files1):
        landmark_numbers = sorted(list(set([f.num_landmarks for f in files0])))
        
        for num_landmarks in landmark_numbers:
            print('Number of landmarks: ' + str(num_landmarks))
            files0_l = [f for f in files0 if f.num_landmarks == num_landmarks]
            files1_l = [f for f in files1 if f.num_landmarks == num_landmarks]
            
            files0_l_sorted, files1_l_sorted = [], []
            for f0 in files0_l:
                for f1 in files1_l:
                    if f0.file_time == f1.file_time:
                        files0_l_sorted.append(f0)
                        files1_l_sorted.append(f1)
            
            sum_cov_trace_pct_data = Data()
            speed_pct_data = Data()
            for f0, f1 in zip(files0_l_sorted, files1_l_sorted):
                for i in xrange(f0.num_examples):
                	sum_cov_trace_pct_data.add(f0.example[i]['sum_cov_trace'] / f1.example[i]['sum_cov_trace'])
                	speed_pct_data.add(f1.example[i]['total_time'] / f0.example[i]['total_time'])
            
            print(f0.slam_type+'/'+f1.slam_type+' sum_cov_trace: ' + str(sum_cov_trace_pct_data.mean*100) + ' +- ' + str(sum_cov_trace_pct_data.sd*100) + '%')
            print(f1.slam_type+'/'+f0.slam_type+' speed: ' + str(speed_pct_data.mean*100) + ' +- ' + str(speed_pct_data.sd*100) + '%')
            print('')
            
# assume one file per landmark number
class FileGroup:
	def __init__(self, files):
		self.files = files
		self.slam_type = self.files[0].slam_type
		
	def getFileWithLandmark(self, num_landmarks):
		for f in self.files:
			if f.num_landmarks == num_landmarks:
				return f
		return None
		
	def getStats(self, num_landmarks, attr):
		f = self.getFileWithLandmark(num_landmarks)
		
		d = Data()
		for i in xrange(f.num_examples):
			if f.example[i]['failure'] == 0:
				d.add(f.example[i][attr])
				
		return d.mean, d.sd
		
	def getCostStats(self, num_landmarks):
		return self.getStats(num_landmarks, 'sum_cov_trace')
		
	def getTimeStats(self, num_landmarks):
		return self.getStats(num_landmarks, 'solve_time')
		
	def compareAttr(self, otherFileGroup, num_landmarks, attr):
		f_self = self.getFileWithLandmark(num_landmarks)
		f_other = otherFileGroup.getFileWithLandmark(num_landmarks)
		
		d = Data()
		for i in xrange(f_self.num_examples):
			if f_self.example[i]['failure'] == 0 and f_other.example[i]['failure'] == 0:
				d.add(f_self.example[i][attr] / f_other.example[i][attr])
				
		return d.mean, d.sd
		
	def compareCost(self, otherFileGroup, num_landmarks):
		return self.compareAttr(otherFileGroup, num_landmarks, 'sum_cov_trace')
		
	def compareTime(self, otherFileGroup, num_landmarks):
		return self.compareAttr(otherFileGroup, num_landmarks, 'total_time')	
        
    
def process_data():
    curr_path = os.path.abspath('.')
    dir_list = os.listdir(curr_path)
    
    data_files = [f for f in dir_list if '.txt' in f]
    
    slam_types = [f.split('.txt')[0].split('_',1)[0] for f in data_files]
    file_times = [f.split('.txt')[0].split('_',1)[1] for f.split('.txt')[0] in data_files]
    
    files = [File(data_file, slam_type, file_time) for data_file, slam_type, file_time in zip(data_files, slam_types, file_times) if slam_type in File.slam_types]
    
    belief_files = sorted([file for file in files if file.slam_type == 'slam-belief'], key=lambda f: f.num_landmarks)
    state_files = sorted([file for file in files if file.slam_type == 'slam-state'], key=lambda f: f.num_landmarks)
    control_files = sorted([file for file in files if file.slam_type == 'slam-control'], key=lambda f: f.num_landmarks)
    traj_files = sorted([file for file in files if file.slam_type == 'slam-traj'], key=lambda f: f.num_landmarks)
    
    #beliefFG = FileGroup(belief_files)
    stateFG = FileGroup(state_files)
    controlFG = FileGroup(control_files)
    trajFG = FileGroup(traj_files)
    
    landmarks = [3,4,5,6]
    
    print('############ Absolute statistics #########')
    for num_landmarks in landmarks:
    	print('Number of landmarks: ' + str(num_landmarks))
    	for fg in [trajFG, stateFG, controlFG]:
    		cost_avg, cost_sd = fg.getCostStats(num_landmarks)
    		time_avg, time_sd = fg.getTimeStats(num_landmarks)
    		
    		print(fg.slam_type)
    		print('Cost: {0} +- {1}'.format(cost_avg, cost_sd))
    		print('Time: {0} +- {1} ms'.format(time_avg, time_sd))
    print('\n')
    
    print('############ Relative statistics #############')
    cost_fig = plt.figure(1)
    cost_ax = cost_fig.add_subplot(111)
    time_fig = plt.figure(2)
    time_ax = time_fig.add_subplot(111)
    
    state_comp_times = []
    control_comp_times = []
    for fg in [stateFG, controlFG]:
    	cost_comp_avgs, cost_comp_sds = [], []
    	time_comp_avgs, time_comp_sds = [], []
        for num_landmarks in landmarks:
            print('Number of landmarks: ' + str(num_landmarks))
            cost_comp_avg, cost_comp_sd = fg.compareCost(trajFG, num_landmarks)
            time_comp_avg, time_comp_sd = fg.compareTime(trajFG, num_landmarks)
            
            cost_comp_avgs.append(cost_comp_avg)
            cost_comp_sds.append(cost_comp_sd)
            time_comp_avgs.append(time_comp_avg)
            time_comp_sds.append(time_comp_sd)
    		
            print(fg.slam_type + ' compared with ' + trajFG.slam_type)
            print('Cost: {0} +- {1}'.format(cost_comp_avg, cost_comp_sd))
            print('Time: {0} +- {1}'.format(time_comp_avg, time_comp_sd))
            
        cost_ax.errorbar(landmarks, cost_comp_avgs, yerr=cost_comp_sds, elinewidth=2, label=fg.slam_type)
        time_ax.errorbar(landmarks, time_comp_avgs, yerr=time_comp_sds, label=fg.slam_type)
            
    for ax in [cost_ax, time_ax]:
        ax.set_xticks(landmarks)
        ax.set_xlabel('Number of landmarks')
        
        handles, labels = ax.get_legend_handles_labels()
        ax.legend(handles, labels)
    
    cost_ax.set_ylabel('Cost factor versus trajectory')
    time_ax.set_ylabel('Time factor versus trajectory')
    
    cost_ax.set_title('Cost factor of belief, state, and control versus trajectory')
    time_ax.set_title('Time factor of belief, state, and control versus trajectory')
    
    plt.show(block=False)
    raw_input()
    		
    return
    
    print('traj_files statistics')
    File.printStatistics(traj_files)
    print('belief_files statistics')
    File.printStatistics(belief_files)
    print('state_files average')
    File.printStatistics(state_files)
    print('control_files average')
    File.printStatistics(control_files)
    
    print('compare belief to traj')
    File.compare(belief_files, traj_files)
    print('compare state to traj')
    File.compare(state_files, traj_files)
    print('compare control to traj')
    File.compare(control_files, traj_files)
    
    landmarks = [f.num_landmarks for f in traj_files]
    
    traj_avg_total_times = np.array([f.getAverage('total_time') for f in traj_files])
    belief_avg_total_times = np.array([f.getAverage('total_time') for f in belief_files])
    state_avg_total_times = np.array([f.getAverage('total_time') for f in state_files])
    control_avg_total_times = np.array([f.getAverage('total_time') for f in control_files])
    
    print('Average total times for 3, 4, 5 landmarks')
    print('Trajectory: ' + str(["%0.2f"%t for t in traj_avg_total_times]))
    print('Belief: ' + str(["%0.2f"%t for t in belief_avg_total_times]))
    print('State: ' + str(["%0.2f"%t for t in state_avg_total_times]))
    print('Control: ' + str(["%0.2f"%t for t in control_avg_total_times]))
    
    
    landmarks = [3,4,5,6,10,15,20,25,30,35,40,45,50]
    
    # speed = (slam_type) / trajectory_speed 
    b_speeds = []
    s_speeds = []
    c_speeds = []
    # line graph
    # x axis: number landmarks
    # y axis: factor speed compared to traj
    for num_landmarks in landmarks:
    	t = [file for file in traj_files if file.num_landmarks == num_landmarks][0]
    	s = [file for file in state_files if file.num_landmarks == num_landmarks][0]
    	c = [file for file in control_files if file.num_landmarks == num_landmarks][0]
    	
    	s_speeds.append(File.compareAttr(s,t,'total_time'))
    	c_speeds.append(File.compareAttr(c,t,'total_time'))
    	
    	if num_landmarks <= 5:
    		b = [file for file in belief_files if file.num_landmarks == num_landmarks][0]
    		b_speeds.append(File.compareAttr(b,t,'total_time'))
    		
    fig = plt.figure()
    ax = fig.add_subplot(111)
    
    ax.set_xticks(landmarks)
    
    ax.set_ylabel('Time factor versus trajectory')
    ax.set_xlabel('Number of landmarks')
    ax.set_title('Time factor of belief, state, and control versus trajectory')
    
    ax.plot(landmarks[:len(b_speeds)], b_speeds, label='belief')
    ax.plot(landmarks, s_speeds, label='state')
    ax.plot(landmarks, c_speeds, label='control')
    
    handles, labels = ax.get_legend_handles_labels()
    ax.legend(handles, labels)
    
    plt.show(block=False)
    raw_input()
    
    	
    b_costs = []
    s_costs = []
    c_costs = []
    # x axis: number landmarks
    # y axis: percent of avg trajectory cost
    for num_landmarks in landmarks:
    	t = [file for file in traj_files if file.num_landmarks == num_landmarks][0]
    	s = [file for file in state_files if file.num_landmarks == num_landmarks][0]
    	c = [file for file in control_files if file.num_landmarks == num_landmarks][0]
    	
    	s_costs.append(File.compareAttr(s,t,'sum_cov_trace'))
    	c_costs.append(File.compareAttr(c,t,'sum_cov_trace'))
    	
    	if num_landmarks <= 5:
    		b = [file for file in belief_files if file.num_landmarks == num_landmarks][0]
    		b_costs.append(File.compareAttr(b,t,'sum_cov_trace'))
    	
    fig = plt.figure()
    ax = fig.add_subplot(111)
    
    ax.set_xticks(landmarks)
    
    ax.set_ylabel('Cost factor versus trajectory')
    ax.set_xlabel('Number of landmarks')
    ax.set_title('Cost factor of belief, state, and control versus trajectory')
    
    ax.plot(landmarks[:len(b_costs)], b_costs, label='belief')
    ax.plot(landmarks, s_costs, label='state')
    ax.plot(landmarks, c_costs, label='control')
    
    handles, labels = ax.get_legend_handles_labels()
    ax.legend(handles, labels)
    
    plt.show(block=False)
    IPython.embed()
    return
    	
    
    for num_landmarks in [3,4,5]:
    	t = [file for file in traj_files if file.num_landmarks == num_landmarks][0]
    	b = [file for file in belief_files if file.num_landmarks == num_landmarks][0]
    	s = [file for file in state_files if file.num_landmarks == num_landmarks][0]
    	c = [file for file in control_files if file.num_landmarks == num_landmarks][0]
    	
    	avg_sum_cov_traces = [f.getAverage('sum_cov_trace') for f in [t,b,s,c]]
    	sd_sum_cov_traces = [f.getSd('sum_cov_trace') for f in [t,b,s,c]]
    	
    	fig = plt.figure()
    	ax = fig.add_subplot(111)
    	
    	ind = np.arange(len(avg_sum_cov_traces))
    	width = 0.35
    	
    	rects = ax.bar(ind, avg_sum_cov_traces, width,
    					yerr=sd_sum_cov_traces,
    					error_kw=dict(elinewidth=2,ecolor='red'))
    	
    	ax.set_xlim(-width,len(ind)+width)
    	ax.set_ylabel('Trace of Covariance')
    	ax.set_title('Average trace of covariance with {0} landmarks over {1} runs'.format(num_landmarks, b.num_examples))
    	xTickMarks = File.slam_types
    	ax.set_xticks(ind+width)
    	xtickNames = ax.set_xticklabels(xTickMarks)
    	
    	plt.show(block=False)
    	raw_input()
    	
    
		
    #IPython.embed()
    

if __name__ == '__main__':
    process_data()
    