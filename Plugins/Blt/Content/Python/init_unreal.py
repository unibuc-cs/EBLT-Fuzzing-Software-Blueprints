# noinspection PyUnresolvedReferences
import unreal as ue

from strgen import StringGenerator


@ue.uclass()
class PythonBridgeImplementation(ue.PythonBridge):
    @ue.ufunction(override=True)
    def generate_string_from_regex(self, regex: str) -> str:
        return StringGenerator(regex).render()


if __name__ == '__main__':
    ue.log_warning("BLT plugin Python bridge has been initiated!")
