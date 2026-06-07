#ifndef __II__
#define __II__

/**
 * @brief Index of Indices or simply, II.
 * 
 * I designed this data stucture to code for the GBA.
 * Fundamentally, it is a preloaded and sorted stack where the user 
 * can pop a currently unused index between 0 and `N` and can push those
 * same values back to maintain a stack of available indices.
 * 
 * @note one improvement would be to use smaller number types or adding a type template
 *       for numerics
 * 
 * @warning This can be misused if you push values between 0 and `N` that
 *          are currently unused. I haven't implemented that checking yet.
 * 
 * @tparam N number of indices to load the stack
 */
template<int N>
class II{
    public:
        II(){
            ct = 0;
            starting_value = 0;
            for(int i = 0; i < N; i++){
                arr[i] = i;
            }
        };
        II(int starting_val){
            ct = 0;
            starting_value = starting_val;
            for(int i = 0; i < N; i++){
                arr[i] = i + starting_val;
            }
        };

        void reset(){
            ct = 0;
            for(int i = 0; i < N; i++){
                arr[i] = i + starting_value;
            }
        }

        int pop(){
            ct++;
            return arr[ct-1];
        };

        // "< ct" = LHS, active sprite indices
        // ">= ct" = RHS, sprite indices stack
        void push(int index){
            int insert_ind = ct - 1;

            if(index > insert_ind || index < 0) return;

            int current_index = 0;
            //find where index currently is in the lhs array
            for(int i = 0; i < ct; i++){
                if(arr[i] == index){
                    current_index = i;
                }
            }

            //if the current ind is less than the insert index,
            //then we must swap the value of the insert index
            //to the current index before replacing insert index
            //with index
            if(current_index < insert_ind){
                arr[current_index] = arr[insert_ind];
            }
            arr[insert_ind] = index;
            
            //sort the newly inserted index to its correct location in the list
            while (arr[insert_ind] > arr[insert_ind+1]) {
                int temp = arr[insert_ind];
                arr[insert_ind] = arr[insert_ind+1];
                arr[insert_ind+1] = temp;
                insert_ind++;
            }
            
            ct--;
        };

        int at(int index){
            return arr[index];
        }

        int size(){
            return ct;
        }

        int arr[N];
    private:
        int ct;
        int starting_value;
};

#endif