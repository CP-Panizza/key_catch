import os    
# from PIL import Image  
def file_name(file_dir):            
	L=[]   
	for root, dirs, files in os.walk(file_dir):               
		for file in files:              
			#L.append(os.path.join(root, file))
			if file.find("@") == 0:
				# if file.find("@"):
				v = file.split("@")
				print(v)
				os.rename(file, v[1])
	return L  


files = file_name("./")

# print(files)

# out_file = open("../data.txt", mode='w')


# for f in files:
# 	name = f.split(".")[0]
# 	# print(name)
# 	I = Image.open(f)
# 	L = I.convert('L')
# 	pixs = L.load()
# 	break
# 	# out_file.write(name)
# 	# out_file.write(" ")
# 	# for width in range(0, L.size[0]-1):
# 	# 	for height in range(0, L.size[1]-1):
# 	# 		print(str(pixs[height,width]) + " ")
# 	# out_file.write("\n")