package main

import (
	"bytes"
	"fmt"
	"image"
	_ "image/color"
	_ "image/gif"
	_ "image/jpeg"
	_ "image/png"
	"io/ioutil"
	"os"
	"strconv"
	"strings"
)

var listFilePrefix string = "  "

func main() {
	floid := "C:\\Users\\cmj\\Desktop\\key_catch\\data\\"
	srcDir := "./"
	pathSeparator := string(os.PathSeparator)
	level := 1
	files := []string{}
	listAllFileByName(level, pathSeparator, srcDir, &files)

	need_file := []string{}
	for _, v := range files {
		if strings.Index(v, ".png") != -1 {
			need_file = append(need_file, v)
		}
	}

	f, err := os.Create("C:\\Users\\cmj\\Desktop\\key_catch\\data.txt")
	defer f.Close()
	if err != nil {
		fmt.Println(err.Error())
		return
	}

	for _, v := range need_file {
		split := strings.Split(v, ".")
		f.Write([]byte(split[0] + " "))
		fmt.Println(floid + v)
		ff, _ := ioutil.ReadFile(floid + v) //读取文件
		bbb := bytes.NewBuffer(ff)
		m, _, _ := image.Decode(bbb)
		bounds := m.Bounds()
		dx := bounds.Dx()
		dy := bounds.Dy()
		fmt.Println(dx, dy)
		for i := 0; i < dy; i++ {
			for j := 0; j < dx; j++ {
				colorRgb := m.At(j, i)
				r, _, _, _ := colorRgb.RGBA()
				r_uint8 := int(r >> 8) //转换为 255 值
				// g_uint8 := uint8(g >> 8)
				// b_uint8 := uint8(b >> 8)
				// a_uint8 := uint8(a >> 8)

				f.Write([]byte(strconv.Itoa(r_uint8) + " "))
			}
		}
		f.Write([]byte("\r\n"))
	}
}

func listAllFileByName(level int, pathSeparator string, fileDir string, f *[]string) {
	files, _ := ioutil.ReadDir(fileDir)

	tmpPrefix := ""
	for i := 1; i < level; i++ {
		tmpPrefix = tmpPrefix + listFilePrefix
	}

	for _, onefile := range files {
		if onefile.IsDir() {
			fmt.Printf("\033[34m %s %s \033[0m \n", tmpPrefix, onefile.Name())
			//fmt.Println(tmpPrefix, onefile.Name(), "目录:")
			listAllFileByName(level+1, pathSeparator, fileDir+pathSeparator+onefile.Name(), f)
		} else {
			//fmt.Println(tmpPrefix, onefile.Name())
			*f = append(*f, onefile.Name())
		}
	}

}
