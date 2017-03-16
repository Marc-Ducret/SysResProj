public class Keycode {

    static int ct = 1;

    public static void main(String[] args) {
        entries();
        System.out.println();
        table("abcdefghijklmnopqrstuvwxyz ");
    }

    static void entries() {
        entry("escape");
        letters("1234567890");
        entry("rpar");
        entry("equal");
        entry("backspace");
        entry("tab");
        letters("azertyuiop");
        entry("hat");
        entry("dollar");
        entry("enter");
        entry("ctrl");
        letters("qsdfghjklm");
        entry("percent");
        entry("square_superscript");
        entry("shift");
        entry("star");
        letters("wxcvbn");
        entry("comma");
        entry("semi_colon");
        entry("colon");
        entry("exclamation");
        entry("rshift");
        entry("num_star");
        entry("alt_gr");
        entry("space");
    }

    static void table(String alphabet) {
        for(int i = 0; i < alphabet.length(); i++) {
            char c = alphabet.charAt(i);
            System.out.println("chars[KEY_"+name(c)+"] = '"+c+"';");
        }
    }

    static String name(char c) {
        if(c == ' ') return "SPACE";
        else return (""+c).toUpperCase();
    }

    static void entry(String name) {
        System.out.println("#define KEY_"+name.toUpperCase()+" "+ct);
        ct++;
    }

    static void letters(String letters) {
        for(int i = 0; i < letters.length(); i ++) {
            entry(""+letters.charAt(i));
        }
    }
}
