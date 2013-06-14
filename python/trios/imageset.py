"""

Format:

[ 
 ['input1', 'ideal1', 'mask1'],
 ['input2', 'ideal2'],
 ....
 ....
]

"""

class Imageset(list):
	
	@staticmethod
	def skip_blank(lines, count):
		while lines[count].strip() == "":
			count += 1
		return count
	
	@classmethod
	def read(cls, fname):
		f = open(fname, 'r')
		imgset = Imageset()
		
		lines = f.readlines()
		
		n = int(lines[1].split()[1])
		ngroups = int(lines[2].split()[1])
		if ngroups != 2 and ngroups != 3:
			print('Number of groups must be 2 or 3')
			return None
		
		base_dir = [0] * 3
		base_dir[0] = lines[4].strip()
		base_dir[1] = lines[5].strip()
		
		if ngroups == 3:
			base_dir[2] = lines[6].strip()
			
		for i in range(3):
			if (base_dir[i].startswith('./') and 
				len(base_dir[i].split('/')) == 2):
				
				base_dir[i] = base_dir[i][2:]
			
		line_count = 7
		for i in range(n):
			line_count = cls.skip_blank(lines, line_count)
			input_img = base_dir[0] + lines[line_count].strip()
			ideal_img = base_dir[1] + lines[line_count+1].strip()
			if ngroups == 2:
				imgset.append( (input_img, ideal_img) )
			else:
				mask_img = base_dir[2] + lines[line_count+2].strip()
				imgset.append( (input_img, ideal_img, mask_img) )
			line_count += ngroups		
		
		f.close()
		return imgset
		
	def append(self, example):
		if len(example) != 2 and len(example) != 3:
			return 
			
		if len(self) == 0:
			super(Imageset, self).append(example)
		elif len(example) == len(self[0]):
			super(Imageset, self).append(example) 
		
	def write(self, fname):
		f = open(fname, 'w')
		f.write('IMGSET  ########################################################\n')
		
		n = len(self)
		ngroups = len(self[0])
		
		f.write('.n %d\n' % n)
		f.write('.f %d\n' % ngroups)
		f.write('.d\n' + './\n' * ngroups + '\n')
		
		for example in self:
			f.write('\n'.join(example))
			f.write('\n\n')
		
		f.close()
		#Show?
	


def read(fname):
	return Imageset.read(fname)
	
