.PHONY := clean
object = client server
all: $(object)

$(object): %: %.c
		gcc $< -o $@ 


.PHONY := clean
clean:
		rm -rf $(object)

