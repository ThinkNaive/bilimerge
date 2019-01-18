# for dir in {1..18};
#     do
#     cd $dir;
#     cd lua.flv720.bili2api.64;
#     for video in *.blv;
#         do
#         mv "$video" "../../${dir}_${video%.blv}.flv";
#         done;
#     cd ..;
#     cd ..;
# done

gcc -o bilimerge bilimerge.c flv.c kmp.c
